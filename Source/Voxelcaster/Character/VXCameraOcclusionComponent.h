// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VXCameraOcclusionComponent.generated.h"

class UMaterialInterface;
class UPrimitiveComponent;

/** 가리는 벽을 어떻게 처리할지 */
UENUM(BlueprintType)
enum class EVXOcclusionMode : uint8
{
	/** 반투명 머티리얼로 바꾼다 (벽 모양은 보이고 너머가 비친다) */
	Translucent,
	/** 화면에서 숨긴다 (그림자·충돌은 유지) */
	Hide
};

/**
 * 탑다운 카메라와 캐릭터 사이를 가리는 벽을 반투명(또는 숨김)으로 만든다.
 * - 카메라 → 캐릭터로 구체를 쓸어 보내 걸리는 메시를 찾는다
 * - 반투명: 모든 머티리얼 슬롯을 FadeMaterial로 바꿨다가, 더 이상 가리지 않으면 원래 머티리얼로 되돌린다
 * - 밟고 있는 바닥과, 발 근처에서 닿은 물체(바닥·계단 디딤판·카펫)는 건드리지 않는다
 * - 충돌은 그대로라 벽을 통과하지는 못한다
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXELCASTER_API UVXCameraOcclusionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVXCameraOcclusionComponent();

	/** 사용 여부 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	EVXOcclusionMode Mode = EVXOcclusionMode::Translucent;

	/** 반투명 모드에서 씌울 머티리얼. 비었거나 불러오지 못하면 숨김 모드로 동작한다 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	TSoftObjectPtr<UMaterialInterface> FadeMaterial;

	/** 검사 구체 반경 (cm). 클수록 캐릭터 주변을 넓게 비운다 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	float ProbeRadius = 80.f;

	/** 구체가 닿은 지점이 캐릭터 발보다 이 높이(cm) 이상일 때만 처리한다 (바닥·계단·카펫 제외) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Camera")
	float MinHeightAboveFeet = 60.f;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetOccluding(UPrimitiveComponent* Component, bool bOccluding);
	void RestoreAll();

	/** 지금 처리 중인 메시 */
	TSet<TWeakObjectPtr<UPrimitiveComponent>> OccludingComponents;

	/** 반투명으로 바꾸기 전 머티리얼 (컴포넌트별, 슬롯 순서) */
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TArray<TWeakObjectPtr<UMaterialInterface>>> SavedMaterials;

	/** 바꿔 둔 동안 원래 머티리얼이 메모리에서 내려가지 않게 붙잡아 둔다 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> KeepAlive;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> LoadedFadeMaterial;
};
