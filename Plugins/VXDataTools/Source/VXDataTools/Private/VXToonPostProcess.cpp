// Copyright Epic Games, Inc. All Rights Reserved.

#include "VXToonPostProcess.h"
#include "Editor.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Materials/MaterialInterface.h"
#include "ScopedTransaction.h"
#include "VXDataReimporter.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	const TCHAR* ToonInstancePath = TEXT("/Game/Voxelcaster/Materials/MI_PP_Toon.MI_PP_Toon");

	void Notify(const FString& Message, bool bSuccess)
	{
		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = bSuccess ? 4.f : 7.f;
		if (TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}

	void ApplySettings(APostProcessVolume* Volume, UMaterialInterface* ToonMaterial)
	{
		// 레벨 전체에 적용
		Volume->bUnbound = true;

		FPostProcessSettings& S = Volume->Settings;

		// 카툰·윤곽선 머티리얼 (이미 있으면 가중치만 1로)
		bool bFound = false;
		for (FWeightedBlendable& Blendable : S.WeightedBlendables.Array)
		{
			if (Blendable.Object == ToonMaterial)
			{
				Blendable.Weight = 1.f;
				bFound = true;
			}
		}
		if (false == bFound)
		{
			S.WeightedBlendables.Array.Add(FWeightedBlendable(1.f, ToonMaterial));
		}

		// 할레이션: 블룸 번짐 중 큰 것(#4~#6)만 붉은 주황
		S.bOverride_BloomMethod = true;
		S.BloomMethod = BM_SOG;
		S.bOverride_BloomIntensity = true;
		S.BloomIntensity = 0.8f;
		S.bOverride_BloomThreshold = true;
		S.BloomThreshold = 1.f;
		const FLinearColor HalationTint(1.f, 0.35f, 0.2f);
		S.bOverride_Bloom4Tint = true;
		S.Bloom4Tint = HalationTint;
		S.bOverride_Bloom5Tint = true;
		S.Bloom5Tint = HalationTint;
		S.bOverride_Bloom6Tint = true;
		S.Bloom6Tint = HalationTint;

		// 필름 그레인: 톤매핑 단계에서 들어가 셀 셰이딩(After Tonemapping) 계단 경계에서 점으로 튄다 → 끈다
		S.bOverride_FilmGrainIntensity = true;
		S.FilmGrainIntensity = 0.f;

		// 노출: 셀 셰이딩은 화면 밝기 기준이라 보정은 0에서 시작한다 (L_Arena 렌더 테스트)
		S.bOverride_AutoExposureBias = true;
		S.AutoExposureBias = 0.f;

		// 색수차: 화면 가장자리만
		S.bOverride_SceneFringeIntensity = true;
		S.SceneFringeIntensity = 0.5f;
		S.bOverride_ChromaticAberrationStartOffset = true;
		S.ChromaticAberrationStartOffset = 0.3f;

		// 비네트
		S.bOverride_VignetteIntensity = true;
		S.VignetteIntensity = 0.3f;
	}
}

namespace VXToonPostProcess
{
	int32 ApplyToCurrentLevel()
	{
		UWorld* World = nullptr != GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (nullptr == World)
		{
			Notify(TEXT("에디터 레벨을 찾지 못했습니다"), false);
			return 0;
		}

		UMaterialInterface* ToonMaterial = LoadObject<UMaterialInterface>(nullptr, ToonInstancePath);
		if (nullptr == ToonMaterial)
		{
			Notify(TEXT("MI_PP_Toon을 찾지 못했습니다. /Game/Voxelcaster/Materials/MI_PP_Toon 이 있는지 확인하세요"), false);
			return 0;
		}

		TArray<APostProcessVolume*> Volumes;
		for (TActorIterator<APostProcessVolume> It(World); It; ++It)
		{
			Volumes.Add(*It);
		}
		if (Volumes.IsEmpty())
		{
			Notify(TEXT("레벨에 Post Process Volume이 없습니다. 하나 배치한 뒤 다시 누르세요"), false);
			return 0;
		}

		// Ctrl+Z로 되돌릴 수 있게 트랜잭션으로 묶는다
		const FScopedTransaction Transaction(FText::FromString(TEXT("카툰 포스트 프로세스 적용")));
		for (APostProcessVolume* Volume : Volumes)
		{
			Volume->Modify();
			ApplySettings(Volume, ToonMaterial);
			Volume->PostEditChange();
			UE_LOG(LogVXDataTools, Log, TEXT("Toon post process applied to %s"), *Volume->GetActorNameOrLabel());
		}

		Notify(FString::Printf(TEXT("Post Process Volume %d개에 카툰 설정 적용 (레벨 저장 필요, 노출은 직접 설정)"), Volumes.Num()), true);
		return Volumes.Num();
	}
}
