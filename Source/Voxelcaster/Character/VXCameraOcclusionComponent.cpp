// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VXCameraOcclusionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Voxelcaster.h"

namespace
{
	TAutoConsoleVariable<int32> CVarDebugOcclusion(
		TEXT("VX.Debug.CameraOcclusion"), 0,
		TEXT("1: 카메라를 가려 숨기거나 다시 보이게 한 메시를 로그에 남긴다"));
}

UVXCameraOcclusionComponent::UVXCameraOcclusionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	FadeMaterial = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Materials/M_VX_OcclusionFade.M_VX_OcclusionFade")));
}

void UVXCameraOcclusionComponent::BeginPlay()
{
	Super::BeginPlay();
	LoadedFadeMaterial = FadeMaterial.IsNull() ? nullptr : FadeMaterial.LoadSynchronous();
	if (EVXOcclusionMode::Translucent == Mode && nullptr == LoadedFadeMaterial)
	{
		UE_LOG(LogVX, Warning, TEXT("CameraOcclusion: fade material '%s' not found, walls will be hidden instead"), *FadeMaterial.ToString());
	}
}

void UVXCameraOcclusionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const APawn* Pawn = Cast<APawn>(GetOwner());
	const UCameraComponent* Camera = nullptr != Pawn ? Pawn->FindComponentByClass<UCameraComponent>() : nullptr;
	if (false == bEnabled || nullptr == Camera || false == Pawn->IsLocallyControlled())
	{
		RestoreAll();
		return;
	}

	const ACharacter* Character = Cast<ACharacter>(Pawn);
	const float HalfHeight = nullptr != Character ? Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;
	const FVector Target = Pawn->GetActorLocation();
	const float FeetZ = Target.Z - HalfHeight;
	const UPrimitiveComponent* StandingOn = nullptr != Character && Character->GetCharacterMovement()
		? Character->GetCharacterMovement()->CurrentFloor.HitResult.GetComponent() : nullptr;

	// 카메라 → 캐릭터 가슴까지 구체로 쓸어 가리는 물체를 모은다 (오브젝트 쿼리는 막힘 없이 전부 반환)
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_WorldStatic);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VXCameraOcclusion), false, Pawn);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, Camera->GetComponentLocation(), Target, FQuat::Identity, Objects, FCollisionShape::MakeSphere(ProbeRadius), Params);

	TSet<TWeakObjectPtr<UPrimitiveComponent>> NowHidden;
	for (const FHitResult& Hit : Hits)
	{
		UPrimitiveComponent* Component = Hit.GetComponent();
		if (nullptr == Component || Component->GetOwner() == nullptr || Component->GetOwner()->IsA<APawn>())
		{
			continue;
		}
		// 움직이는 물체(탄환, 문 등)는 건드리지 않는다. 벽·기둥 같은 고정 메시만
		if (EComponentMobility::Movable == Component->Mobility)
		{
			continue;
		}
		// 밟고 있는 바닥(계단 포함)은 숨기지 않는다
		if (Component == StandingOn)
		{
			continue;
		}
		// 구체가 닿은 지점이 발 근처(바닥·계단 디딤판·카펫)면 제외. 벽은 높은 곳에서 닿는다
		if (Hit.ImpactPoint.Z < FeetZ + MinHeightAboveFeet)
		{
			continue;
		}
		NowHidden.Add(Component);
	}

	// 더 이상 가리지 않는 것은 다시 보이게, 새로 가리는 것은 숨긴다
	for (const TWeakObjectPtr<UPrimitiveComponent>& Previous : OccludingComponents)
	{
		if (false == NowHidden.Contains(Previous) && Previous.IsValid())
		{
			SetOccluding(Previous.Get(), false);
		}
	}
	for (const TWeakObjectPtr<UPrimitiveComponent>& Current : NowHidden)
	{
		if (false == OccludingComponents.Contains(Current))
		{
			SetOccluding(Current.Get(), true);
		}
	}
	OccludingComponents = MoveTemp(NowHidden);
}

void UVXCameraOcclusionComponent::SetOccluding(UPrimitiveComponent* Component, bool bOccluding)
{
	if (nullptr == Component)
	{
		return;
	}

	UMeshComponent* Mesh = Cast<UMeshComponent>(Component);
	const bool bTranslucent = EVXOcclusionMode::Translucent == Mode && nullptr != LoadedFadeMaterial && nullptr != Mesh;

	if (bTranslucent)
	{
		if (bOccluding)
		{
			// 원래 머티리얼을 기억하고 모든 슬롯을 반투명 머티리얼로
			TArray<TWeakObjectPtr<UMaterialInterface>>& Saved = SavedMaterials.FindOrAdd(Component);
			Saved.Reset();
			for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
			{
				UMaterialInterface* Original = Mesh->GetMaterial(Index);
				Saved.Add(Original);
				KeepAlive.AddUnique(Original);
				Mesh->SetMaterial(Index, LoadedFadeMaterial);
			}
		}
		else if (TArray<TWeakObjectPtr<UMaterialInterface>>* Saved = SavedMaterials.Find(Component))
		{
			for (int32 Index = 0; Index < Saved->Num(); ++Index)
			{
				Mesh->SetMaterial(Index, (*Saved)[Index].Get());
			}
			SavedMaterials.Remove(Component);
		}
	}
	else
	{
		// 메인 패스에서만 빼서 그림자와 충돌은 유지한다
		Component->SetRenderInMainPass(false == bOccluding);
	}

	if (CVarDebugOcclusion.GetValueOnGameThread() > 0)
	{
		UE_LOG(LogVX, Log, TEXT("CameraOcclusion: %s %s (%s)"), bOccluding ? TEXT("occlude") : TEXT("restore"),
			*Component->GetOwner()->GetName(), bTranslucent ? TEXT("translucent") : TEXT("hide"));
	}
}

void UVXCameraOcclusionComponent::RestoreAll()
{
	for (const TWeakObjectPtr<UPrimitiveComponent>& Previous : OccludingComponents)
	{
		if (Previous.IsValid())
		{
			SetOccluding(Previous.Get(), false);
		}
	}
	OccludingComponents.Reset();
	SavedMaterials.Reset();
	KeepAlive.Reset();
}

void UVXCameraOcclusionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreAll();
	Super::EndPlay(EndPlayReason);
}
