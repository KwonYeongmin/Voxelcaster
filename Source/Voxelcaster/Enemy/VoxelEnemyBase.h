// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VoxelCharacterBase.h"
#include "VoxelEnemyBase.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStateTree;
class UAnimMontage;
class UWidgetComponent;
class AVXEnemyBase;

/** 적이 다른 적을 소환했을 때 (웨이브 매니저가 클리어 조건에 포함시킨다) */
DECLARE_MULTICAST_DELEGATE_OneParam(FVXMinionSpawnedSignature, AVXEnemyBase* /*Minion*/);

/**
 * 적 공통 베이스 적 진영, 임시 큐브 메시, 플레이어 탐색, 사망 처리를 제공한다.
 * 이동을 위해 AI 컨트롤러가 자동으로 빙의한다. (컨트롤러가 없으면 캐릭터 이동이 동작하지 않는다)
 */
UCLASS(Abstract)
class VOXELCASTER_API AVXEnemyBase : public AVXCharacterBase
{
	GENERATED_BODY()

public:
	AVXEnemyBase();

	FVXMinionSpawnedSignature OnMinionSpawned;

	/** 이 적의 행동을 구동하는 StateTree 에셋. 비어 있으면 적 클래스의 기본 행동을 쓴다. */
	const TSoftObjectPtr<UStateTree>& GetStateTreeAsset() const { return StateTreeAsset; }

	/** AI 컨트롤러가 StateTree를 실행 중인지 */
	bool IsDrivenByStateTree() const;

protected:
	virtual void BeginPlay() override;
	virtual void HandleDeath() override;

	/** 살아 있는 플레이어 캐릭터. 없으면 nullptr */
	AVXCharacterBase* FindLivePlayer() const;

	/** 몸 색상을 바꾼다 (예고 연출 등). 임시 큐브 메시를 쓸 때만 동작한다 */
	void SetBodyColor(const FLinearColor& Color);

	/** 블루프린트에서 스켈레탈 메시를 지정했는지 (임시 큐브 대신 실제 캐릭터를 쓰는지) */
	bool HasSkeletalMesh() const;

	/** 임시 메시 색상 (복셀 에셋 확정 전) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FLinearColor BodyColor = FLinearColor(0.8f, 0.1f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FVector BodyScale = FVector(0.6f, 0.6f, 1.6f);

	/** StateTree 에셋 경로 (자식 클래스 생성자에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|AI")
	TSoftObjectPtr<UStateTree> StateTreeAsset;

	/**
	 * 사망 애니메이션. 지정하면 사망 시 재생하고, 끝날 때까지 시체를 남긴다.
	 * 블루프린트에서 스켈레탈 메시(Mesh)를 지정하면 임시 큐브(BodyMesh)는 자동으로 숨겨진다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** 사망 후 제거까지 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Enemy")
	float DeathLifeSpan = 0.2f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** 머리 위 HP 바 (화면 공간 빌보드) */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	void UpdateHealthBar(float Current, float Max);
};
