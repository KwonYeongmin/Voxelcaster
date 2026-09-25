// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VoxelCharacterBase.h"
#include "VoxelEnemyBase.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStateTree;
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

	/** 몸 색상을 바꾼다 (예고 연출 등). 복셀 에셋 확정 전 임시 표현 */
	void SetBodyColor(const FLinearColor& Color);

	/** 임시 메시 색상 (복셀 에셋 확정 전) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FLinearColor BodyColor = FLinearColor(0.8f, 0.1f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FVector BodyScale = FVector(0.6f, 0.6f, 1.6f);

	/** StateTree 에셋 경로 (자식 클래스 생성자에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|AI")
	TSoftObjectPtr<UStateTree> StateTreeAsset;

	/** 사망 후 제거까지 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Enemy")
	float DeathLifeSpan = 0.2f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;
};
