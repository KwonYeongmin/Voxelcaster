// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VXCameraOcclusionComponent.generated.h"

class UPrimitiveComponent;

/**
 * 탑다운 카메라와 캐릭터 사이를 가리는 벽을 화면에서만 숨긴다.
 * - 카메라 → 캐릭터로 구체를 쓸어 보내 걸리는 정적 메시를 찾는다
 * - 숨긴 메시는 화면(메인 패스)에만 안 그려지고 그림자·충돌은 그대로 남는다
 * - 밟고 있는 바닥과, 발 근처에서 닿은 물체(바닥·계단 디딤판·카펫)는 숨기지 않는다
 * - 더 이상 가리지 않으면 다시 보인다
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXELCASTER_API UVXCameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVXCameraOcclusionComponent();

	/** 숨기기 사용 여부 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	bool bEnabled = true;

	/** 검사 구체 반경 (cm). 클수록 캐릭터 주변을 넓게 비운다 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	float ProbeRadius = 80.f;

	/** 구체가 닿은 지점이 캐릭터 발보다 이 높이(cm) 이상일 때만 숨긴다 (바닥·계단·카펫 제외) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	float MinHeightAboveFeet = 60.f;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetHidden(UPrimitiveComponent* Component, bool bHide);
	void RestoreAll();

	/** 지금 숨겨 둔 메시 */
	TSet<TWeakObjectPtr<UPrimitiveComponent>> HiddenComponents;
};
