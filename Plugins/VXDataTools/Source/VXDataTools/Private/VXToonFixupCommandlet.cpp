// Copyright Epic Games, Inc. All Rights Reserved.

#include "VXToonFixupCommandlet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "FileHelpers.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Engine/SkeletalMesh.h"
#include "ReferenceSkeleton.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionDotProduct.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMax.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionSceneTexture.h"
#include "VXDataReimporter.h"
#include "MaterialShared.h"
#include "RHIShaderPlatform.h"

namespace
{
	const TCHAR* ToonMaterialPath = TEXT("/Game/Voxelcaster/Materials/M_PP_Toon.M_PP_Toon");

	bool IsSameInput(const FExpressionInput& A, const FExpressionInput& B)
	{
		return nullptr != A.Expression && A.Expression == B.Expression && A.OutputIndex == B.OutputIndex;
	}

	template <typename T>
	T* Create(UMaterial* Material, int32 X, int32 Y)
	{
		return Cast<T>(UMaterialEditingLibrary::CreateMaterialExpression(Material, T::StaticClass(), X, Y));
	}
}

namespace
{
	/**
	 * 캐릭터가 벽 뒤에 가려진 픽셀(Custom Depth가 장면 깊이보다 앞)에 실루엣 색을 섞는다.
	 * 스텐실 1 = 플레이어 색, 2 = 적 색. 가려지지 않은 캐릭터는 그대로.
	 */
	int32 AddSilhouette(UMaterial* Material)
	{
		UMaterialEditorOnlyData* EditorData = Material->GetEditorOnlyData();
		FExpressionInput& Emissive = EditorData->EmissiveColor;
		if (nullptr == Emissive.Expression)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("Silhouette: Emissive Color is not connected"));
			return 1;
		}
		for (UMaterialExpression* Expression : Material->GetExpressions())
		{
			if (Expression->Desc == TEXT("VX Silhouette"))
			{
				UE_LOG(LogVXDataTools, Display, TEXT("Silhouette: already added. Nothing to do."));
				return 0;
			}
		}

		const FExpressionInput Final = Emissive;
		const int32 X = Emissive.Expression->MaterialExpressionEditorX + 300;
		const int32 Y = Emissive.Expression->MaterialExpressionEditorY + 400;

		Material->PreEditChange(nullptr);

		auto MakeSceneR = [&](ESceneTextureId Id, int32 PosY) -> UMaterialExpression*
		{
			UMaterialExpressionSceneTexture* Texture = Create<UMaterialExpressionSceneTexture>(Material, X, PosY);
			Texture->SceneTextureId = Id;
			UMaterialExpressionComponentMask* Mask = Create<UMaterialExpressionComponentMask>(Material, X + 220, PosY);
			Mask->R = 1; Mask->G = 0; Mask->B = 0; Mask->A = 0;
			Mask->Input.Connect(0, Texture);
			return Mask;
		};

		UMaterialExpression* CustomDepth = MakeSceneR(PPI_CustomDepth, Y);
		UMaterialExpression* SceneDepth = MakeSceneR(PPI_SceneDepth, Y + 150);
		UMaterialExpression* Stencil = MakeSceneR(PPI_CustomStencil, Y + 300);

		UMaterialExpressionScalarParameter* Bias = Create<UMaterialExpressionScalarParameter>(Material, X + 220, Y + 450);
		Bias->ParameterName = TEXT("SilhouetteDepthBias");
		Bias->DefaultValue = 20.f;

		// 캐릭터 깊이 < 장면 깊이 - Bias 이면 벽 뒤 (1), 아니면 0
		UMaterialExpressionSubtract* SceneMinusBias = Create<UMaterialExpressionSubtract>(Material, X + 440, Y + 150);
		SceneMinusBias->A.Connect(0, SceneDepth);
		SceneMinusBias->B.Connect(0, Bias);
		UMaterialExpressionIf* Behind = Create<UMaterialExpressionIf>(Material, X + 660, Y);
		Behind->A.Connect(0, CustomDepth);
		Behind->B.Connect(0, SceneMinusBias);
		UMaterialExpressionConstant* Zero = Create<UMaterialExpressionConstant>(Material, X + 440, Y + 750);
		Zero->R = 0.f;
		Behind->AGreaterThanB.Connect(0, Zero);
		Behind->AEqualsB.Connect(0, Zero);
		UMaterialExpressionScalarParameter* Opacity = Create<UMaterialExpressionScalarParameter>(Material, X + 440, Y + 600);
		Opacity->ParameterName = TEXT("SilhouetteOpacity");
		Opacity->DefaultValue = 0.7f;
		Behind->ALessThanB.Connect(0, Opacity);

		// 스텐실 2 이상 = 적
		UMaterialExpressionSubtract* StencilMinusOne = Create<UMaterialExpressionSubtract>(Material, X + 440, Y + 300);
		StencilMinusOne->A.Connect(0, Stencil);
		StencilMinusOne->ConstB = 1.f;
		UMaterialExpressionSaturate* IsEnemy = Create<UMaterialExpressionSaturate>(Material, X + 660, Y + 300);
		IsEnemy->Input.Connect(0, StencilMinusOne);

		UMaterialExpressionVectorParameter* PlayerColor = Create<UMaterialExpressionVectorParameter>(Material, X + 660, Y + 450);
		PlayerColor->ParameterName = TEXT("SilhouettePlayerColor");
		PlayerColor->DefaultValue = FLinearColor(0.35f, 0.85f, 1.f);
		UMaterialExpressionVectorParameter* EnemyColor = Create<UMaterialExpressionVectorParameter>(Material, X + 660, Y + 600);
		EnemyColor->ParameterName = TEXT("SilhouetteEnemyColor");
		EnemyColor->DefaultValue = FLinearColor(1.f, 0.3f, 0.25f);

		UMaterialExpressionLinearInterpolate* Color = Create<UMaterialExpressionLinearInterpolate>(Material, X + 900, Y + 450);
		Color->A.Connect(0, PlayerColor);
		Color->B.Connect(0, EnemyColor);
		Color->Alpha.Connect(0, IsEnemy);
		UMaterialExpressionComponentMask* ColorRGB = Create<UMaterialExpressionComponentMask>(Material, X + 1100, Y + 450);
		ColorRGB->R = 1; ColorRGB->G = 1; ColorRGB->B = 1; ColorRGB->A = 0;
		ColorRGB->Input.Connect(0, Color);

		UMaterialExpressionLinearInterpolate* Out = Create<UMaterialExpressionLinearInterpolate>(Material, X + 1300, Y);
		Out->A = Final;
		Out->B.Connect(0, ColorRGB);
		Out->Alpha.Connect(0, Behind);
		Out->Desc = TEXT("VX Silhouette");

		Emissive.Connect(0, Out);

		Material->PostEditChange();
		Material->MarkPackageDirty();
		UMaterialEditingLibrary::RecompileMaterial(Material);
		const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ Material->GetPackage() }, false);
		UE_LOG(LogVXDataTools, Display, TEXT("Silhouette: added and saved = %d"), bSaved ? 1 : 0);
		return bSaved ? 0 : 1;
	}
}

namespace
{
	/** 탑다운 카메라를 가리는 벽에 씌우는 반투명 머티리얼 (UVXCameraOcclusionComponent) */
	int32 MakeOcclusionFadeMaterial()
	{
		const TCHAR* PackagePath = TEXT("/Game/Voxelcaster/Materials/M_VX_OcclusionFade");
		if (nullptr != LoadObject<UMaterial>(nullptr, TEXT("/Game/Voxelcaster/Materials/M_VX_OcclusionFade.M_VX_OcclusionFade")))
		{
			UE_LOG(LogVXDataTools, Display, TEXT("MakeFade: already exists. Nothing to do."));
			return 0;
		}

		UPackage* Package = CreatePackage(PackagePath);
		UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();
		UMaterial* Material = Cast<UMaterial>(Factory->FactoryCreateNew(UMaterial::StaticClass(), Package, TEXT("M_VX_OcclusionFade"),
			RF_Public | RF_Standalone | RF_Transactional, nullptr, GWarn));
		if (nullptr == Material)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("MakeFade: failed to create material"));
			return 1;
		}
		FAssetRegistryModule::AssetCreated(Material);

		Material->PreEditChange(nullptr);
		Material->BlendMode = BLEND_Translucent;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;

		UMaterialExpressionVectorParameter* Color = Create<UMaterialExpressionVectorParameter>(Material, -400, 0);
		Color->ParameterName = TEXT("FadeColor");
		Color->DefaultValue = FLinearColor(0.10f, 0.08f, 0.16f);
		UMaterialExpressionComponentMask* ColorRGB = Create<UMaterialExpressionComponentMask>(Material, -200, 0);
		ColorRGB->R = 1; ColorRGB->G = 1; ColorRGB->B = 1; ColorRGB->A = 0;
		ColorRGB->Input.Connect(0, Color);

		UMaterialExpressionScalarParameter* Opacity = Create<UMaterialExpressionScalarParameter>(Material, -400, 200);
		Opacity->ParameterName = TEXT("FadeOpacity");
		Opacity->DefaultValue = 0.3f;

		UMaterialEditorOnlyData* EditorData = Material->GetEditorOnlyData();
		EditorData->EmissiveColor.Connect(0, ColorRGB);
		EditorData->Opacity.Connect(0, Opacity);

		Material->PostEditChange();
		Material->MarkPackageDirty();
		UMaterialEditingLibrary::RecompileMaterial(Material);
		const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ Package }, false);
		UE_LOG(LogVXDataTools, Display, TEXT("MakeFade: created M_VX_OcclusionFade, saved = %d"), bSaved ? 1 : 0);
		return bSaved ? 0 : 1;
	}
}

UVXToonFixupCommandlet::UVXToonFixupCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UVXToonFixupCommandlet::Main(const FString& Params)
{
	// -makefade: 벽 반투명용 머티리얼 M_VX_OcclusionFade를 만든다 (이미 있으면 건너뜀)
	if (Params.Contains(TEXT("-makefade")))
	{
		return MakeOcclusionFadeMaterial();
	}

	// -reimportdata: 데이터 폴더의 DataTable을 원본 CSV/JSON에서 전부 다시 읽고 저장한다 (에디터 버튼과 같은 동작)
	if (Params.Contains(TEXT("-reimportdata")))
	{
		int32 Failed = 0;
		for (const TSharedPtr<FVXDataTableEntry>& Entry : VXDataReimporter::GatherEntries())
		{
			const bool bOk = VXDataReimporter::Reimport(*Entry);
			Failed += bOk ? 0 : 1;
			UE_LOG(LogVXDataTools, Display, TEXT("ReimportData: %s -> %s"), *Entry->AssetName, *Entry->Status);
		}
		return 0 == Failed ? 0 : 1;
	}

	// -meshinfo=/Game/...: 스켈레탈 메시의 크기·중심·루트 뼈를 출력한다 (BP 메시 위치 보정값 계산용)
	FString MeshPath;
	if (FParse::Value(*Params, TEXT("-meshinfo="), MeshPath))
	{
		USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
		if (nullptr == Mesh)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("MeshInfo: %s not found"), *MeshPath);
			return 1;
		}
		const FBoxSphereBounds Bounds = Mesh->GetImportedBounds();
		const FBox Box = Bounds.GetBox();
		UE_LOG(LogVXDataTools, Display, TEXT("MeshInfo: bounds min %s max %s"), *Box.Min.ToString(), *Box.Max.ToString());
		UE_LOG(LogVXDataTools, Display, TEXT("MeshInfo: center %s size %s"), *Box.GetCenter().ToString(), *Box.GetSize().ToString());
		const FReferenceSkeleton& Skeleton = Mesh->GetRefSkeleton();
		for (int32 i = 0; i < FMath::Min(Skeleton.GetNum(), 6); ++i)
		{
			const FTransform& Bone = Skeleton.GetRefBonePose()[i];
			UE_LOG(LogVXDataTools, Display, TEXT("MeshInfo: bone %d %s parent %d loc %s rot %s scale %s"), i, *Skeleton.GetBoneName(i).ToString(),
				Skeleton.GetParentIndex(i), *Bone.GetLocation().ToString(), *Bone.Rotator().ToString(), *Bone.GetScale3D().ToString());
		}
		UE_LOG(LogVXDataTools, Display, TEXT("MeshInfo: bones %d"), Skeleton.GetNum());
		return 0;
	}

	UMaterial* Material = LoadObject<UMaterial>(nullptr, ToonMaterialPath);
	if (nullptr == Material)
	{
		UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup: %s not found"), ToonMaterialPath);
		return 1;
	}

	// -fixmi: MI_PP_Toon의 부모를 M_PP_Toon으로 되돌린다 (파라미터 덮어쓰기 값은 이름이 같으면 유지된다)
	if (Params.Contains(TEXT("-fixmi")))
	{
		UMaterialInstanceConstant* Instance = LoadObject<UMaterialInstanceConstant>(nullptr, TEXT("/Game/Voxelcaster/Materials/MI_PP_Toon.MI_PP_Toon"));
		if (nullptr == Instance)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup fixmi: MI_PP_Toon not found"));
			return 1;
		}
		UE_LOG(LogVXDataTools, Display, TEXT("ToonFixup fixmi: current parent = %s"), nullptr != Instance->Parent ? *Instance->Parent->GetPathName() : TEXT("None"));
		Instance->SetParentEditorOnly(Material);
		Instance->PostEditChange();
		Instance->MarkPackageDirty();
		const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ Instance->GetPackage() }, false);
		UE_LOG(LogVXDataTools, Display, TEXT("ToonFixup fixmi: parent set to M_PP_Toon, saved = %d"), bSaved ? 1 : 0);
		return bSaved ? 0 : 1;
	}

	// -silhouette: 최종 출력(Emissive) 앞에 "벽 뒤 캐릭터 실루엣"을 끼운다
	if (Params.Contains(TEXT("-silhouette")))
	{
		return AddSilhouette(Material);
	}

	// -check: 컴파일 에러만 출력한다
	if (Params.Contains(TEXT("-check")))
	{
		Material->ForceRecompileForRendering();
		FMaterialResource* Resource = Material->GetMaterialResource(GMaxRHIShaderPlatform);
		if (nullptr == Resource)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup check: no material resource"));
			return 1;
		}
		Resource->FinishCompilation();
		const TArray<FString>& Errors = Resource->GetCompileErrors();
		for (const FString& Error : Errors)
		{
			UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup check: %s"), *Error);
		}
		UE_LOG(LogVXDataTools, Display, TEXT("ToonFixup check: %d compile error(s), platform %s"), Errors.Num(), *LexToString(GMaxRHIShaderPlatform));
		return Errors.IsEmpty() ? 0 : 1;
	}

	// ---- 1. 휘도 가중치 Constant3Vector와, 거기 연결된 밝기 Dot 찾기
	UMaterialExpressionConstant3Vector* Weights = nullptr;
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		UMaterialExpressionConstant3Vector* Const3 = Cast<UMaterialExpressionConstant3Vector>(Expression);
		if (Const3 && FMath::IsNearlyEqual(Const3->Constant.R, 0.2126f, 0.001f) && FMath::IsNearlyEqual(Const3->Constant.G, 0.7152f, 0.001f))
		{
			Weights = Const3;
			break;
		}
	}
	if (nullptr == Weights)
	{
		UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup: luminance weights Constant3Vector (0.2126, 0.7152, 0.0722) not found"));
		return 1;
	}

	UMaterialExpressionDotProduct* LumDot = nullptr;
	FExpressionInput* SceneInput = nullptr;
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		UMaterialExpressionDotProduct* Dot = Cast<UMaterialExpressionDotProduct>(Expression);
		if (nullptr == Dot)
		{
			continue;
		}
		if (Dot->B.Expression == Weights && Dot->A.Expression)
		{
			LumDot = Dot;
			SceneInput = &Dot->A;
			break;
		}
		if (Dot->A.Expression == Weights && Dot->B.Expression)
		{
			LumDot = Dot;
			SceneInput = &Dot->B;
			break;
		}
	}
	if (nullptr == LumDot)
	{
		UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup: Dot connected to the luminance weights not found"));
		return 1;
	}
	if (SceneInput->Expression->IsA<UMaterialExpressionDivide>())
	{
		UE_LOG(LogVXDataTools, Display, TEXT("ToonFixup: already applied (Dot input is a Divide). Nothing to do."));
		return 0;
	}
	const FExpressionInput SceneColor = *SceneInput;

	// ---- 2. Toon Multiply 찾기: 한쪽은 SceneColor, 다른 쪽은 Ratio(Divide)
	UMaterialExpressionMultiply* ToonMultiply = nullptr;
	FExpressionInput* RatioInput = nullptr;
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		UMaterialExpressionMultiply* Multiply = Cast<UMaterialExpressionMultiply>(Expression);
		if (nullptr == Multiply)
		{
			continue;
		}
		if (IsSameInput(Multiply->A, SceneColor) && Multiply->B.Expression && Multiply->B.Expression->IsA<UMaterialExpressionDivide>())
		{
			ToonMultiply = Multiply;
			RatioInput = &Multiply->B;
			break;
		}
		if (IsSameInput(Multiply->B, SceneColor) && Multiply->A.Expression && Multiply->A.Expression->IsA<UMaterialExpressionDivide>())
		{
			ToonMultiply = Multiply;
			RatioInput = &Multiply->A;
			break;
		}
	}
	if (nullptr == ToonMultiply)
	{
		UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup: Toon Multiply (SceneColor x Ratio) not found"));
		return 1;
	}
	const FExpressionInput Ratio = *RatioInput;

	Material->PreEditChange(nullptr);

	// ---- 3. Light = SceneColor / Max(BaseColor, 0.01)  → 밝기 Dot의 입력으로
	const int32 X = LumDot->MaterialExpressionEditorX;
	const int32 Y = LumDot->MaterialExpressionEditorY;

	UMaterialExpressionSceneTexture* BaseColor = Create<UMaterialExpressionSceneTexture>(Material, X - 760, Y + 360);
	BaseColor->SceneTextureId = PPI_BaseColor;
	BaseColor->Desc = TEXT("Albedo (텍스처 색)");

	UMaterialExpressionComponentMask* AlbedoMask = Create<UMaterialExpressionComponentMask>(Material, X - 500, Y + 360);
	AlbedoMask->R = 1;
	AlbedoMask->G = 1;
	AlbedoMask->B = 1;
	AlbedoMask->A = 0;
	AlbedoMask->Input.Connect(0, BaseColor);

	UMaterialExpressionMax* SafeAlbedo = Create<UMaterialExpressionMax>(Material, X - 330, Y + 360);
	SafeAlbedo->A.Connect(0, AlbedoMask);
	SafeAlbedo->ConstB = 0.01f;

	UMaterialExpressionDivide* Light = Create<UMaterialExpressionDivide>(Material, X - 170, Y + 220);
	Light->A = SceneColor;
	Light->B.Connect(0, SafeAlbedo);
	Light->Desc = TEXT("Light (빛의 양)");

	SceneInput->Connect(0, Light);

	// ---- 4. HasAlbedo = Saturate(Dot(Albedo, 가중치) × 100), Ratio' = Lerp(1, Ratio, HasAlbedo)
	UMaterialExpressionDotProduct* AlbedoLum = Create<UMaterialExpressionDotProduct>(Material, X - 170, Y + 520);
	AlbedoLum->A.Connect(0, AlbedoMask);
	AlbedoLum->B.Connect(0, Weights);

	UMaterialExpressionMultiply* AlbedoScale = Create<UMaterialExpressionMultiply>(Material, X + 30, Y + 520);
	AlbedoScale->A.Connect(0, AlbedoLum);
	AlbedoScale->ConstB = 100.f;

	UMaterialExpressionSaturate* HasAlbedo = Create<UMaterialExpressionSaturate>(Material, X + 200, Y + 520);
	HasAlbedo->Input.Connect(0, AlbedoScale);
	HasAlbedo->Desc = TEXT("HasAlbedo");

	UMaterialExpressionLinearInterpolate* SafeRatio = Create<UMaterialExpressionLinearInterpolate>(Material,
		ToonMultiply->MaterialExpressionEditorX - 220, ToonMultiply->MaterialExpressionEditorY + 220);
	SafeRatio->ConstA = 1.f;
	SafeRatio->B = Ratio;
	SafeRatio->Alpha.Connect(0, HasAlbedo);
	SafeRatio->Desc = TEXT("Ratio' (하늘·발광면은 1)");

	RatioInput->Connect(0, SafeRatio);

	Material->PostEditChange();
	Material->MarkPackageDirty();

	UMaterialEditingLibrary::RecompileMaterial(Material);

	if (false == UEditorLoadingAndSavingUtils::SavePackages({ Material->GetPackage() }, false))
	{
		UE_LOG(LogVXDataTools, Error, TEXT("ToonFixup: failed to save %s"), ToonMaterialPath);
		return 1;
	}

	UE_LOG(LogVXDataTools, Display, TEXT("ToonFixup: applied and saved (8 nodes added: BaseColor, Mask, Max, Divide, Dot, Multiply, Saturate, Lerp)"));
	return 0;
}
