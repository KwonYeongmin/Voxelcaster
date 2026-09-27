// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VXCameraOcclusionComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
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
	for (const TWeakObjectPtr<UPrimitiveComponent>& Previous : HiddenComponents)
	{
		if (false == NowHidden.Contains(Previous) && Previous.IsValid())
		{
			SetHidden(Previous.Get(), false);
		}
	}
	for (const TWeakObjectPtr<UPrimitiveComponent>& Current : NowHidden)
	{
		if (false == HiddenComponents.Contains(Current))
		{
			SetHidden(Current.Get(), true);
		}
	}
	HiddenComponents = MoveTemp(NowHidden);
}

void UVXCameraOcclusionComponent::SetHidden(UPrimitiveComponent* Component, bool bHide)
{
	if (Component)
	{
		// 메인 패스에서만 빼서 그림자와 충돌은 유지한다
		Component->SetRenderInMainPass(false == bHide);
		if (CVarDebugOcclusion.GetValueOnGameThread() > 0)
		{
			UE_LOG(LogVX, Log, TEXT("CameraOcclusion: %s %s"), bHide ? TEXT("hide") : TEXT("show"), *Component->GetOwner()->GetName());
		}
	}
}

void UVXCameraOcclusionComponent::RestoreAll()
{
	for (const TWeakObjectPtr<UPrimitiveComponent>& Previous : HiddenComponents)
	{
		if (Previous.IsValid())
		{
			SetHidden(Previous.Get(), false);
		}
	}
	HiddenComponents.Reset();
}

void UVXCameraOcclusionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreAll();
	Super::EndPlay(EndPlayReason);
}
