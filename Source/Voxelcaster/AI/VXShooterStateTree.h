// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Conditions/StateTreeAIConditionBase.h"
#include "Tasks/StateTreeAITask.h"
#include "VXShooterStateTree.generated.h"

class AAIController;

/**
 * 슈터 StateTree용 태스크와 조건 (DES-AI-001).
 * 이 노드들은 슈터의 공개 함수를 부르기만 한다. 거리·시야·사격 로직은 AVXShooter에 있다.
 *
 * ST_Shooter 구성 (스키마: StateTree AI Component)
 *   Root
 *   ├─ Approach : 태스크 VX Shooter Move (Approach)
 *   │    전환: CanHoldFire → Hold
 *   ├─ Hold     : 태스크 VX Shooter Fire
 *   │    전환: ShouldRetreat → Retreat,  ShouldApproach → Approach
 *   └─ Retreat  : 태스크 VX Shooter Move (Retreat)
 *        전환: RetreatDone → Hold
 * 전환은 모두 "On Tick"에서 검사하고, 순서대로 먼저 만족하는 전환이 적용된다.
 */

UENUM()
enum class EVXShooterMoveMode : uint8
{
	/** 플레이어를 향해 이동 */
	Approach,
	/** 플레이어 반대 방향으로 이동 */
	Retreat
};

UENUM()
enum class EVXShooterRangeCheck : uint8
{
	/** 사격 가능 거리(최대 유지 거리 안)이고 시야가 트여 있다 */
	CanHoldFire,
	/** 너무 멀거나 시야가 막혔다 */
	ShouldApproach,
	/** 너무 가깝다 */
	ShouldRetreat,
	/** 후퇴 완료 (최소 유지 거리 이상) */
	RetreatDone
};

// ---------------------------------------------------------------------------
// 이동 태스크
// ---------------------------------------------------------------------------

USTRUCT()
struct FVXShooterMoveTaskInstanceData
{
	GENERATED_BODY()

	/** StateTree AI 스키마의 컨텍스트에 자동으로 바인딩된다 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = Parameter)
	EVXShooterMoveMode Mode = EVXShooterMoveMode::Approach;
};

/** 플레이어를 향해 다가가거나 물러난다. 상태에 있는 동안 계속 실행된다. */
USTRUCT(meta = (DisplayName = "VX Shooter Move", Category = "VX|Shooter"))
struct VOXELCASTER_API FVXShooterMoveTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FVXShooterMoveTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};

// ---------------------------------------------------------------------------
// 사격 태스크
// ---------------------------------------------------------------------------

USTRUCT()
struct FVXShooterFireTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> AIController = nullptr;
};

/** 정지한 채 플레이어를 바라보며 주기적으로 사격한다. 상태를 벗어나면 발사 예고를 취소한다. */
USTRUCT(meta = (DisplayName = "VX Shooter Fire", Category = "VX|Shooter"))
struct VOXELCASTER_API FVXShooterFireTask : public FStateTreeAIActionTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FVXShooterFireTaskInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

// ---------------------------------------------------------------------------
// 거리 조건
// ---------------------------------------------------------------------------

USTRUCT()
struct FVXShooterRangeConditionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = Parameter)
	EVXShooterRangeCheck Check = EVXShooterRangeCheck::CanHoldFire;
};

/** 플레이어와의 거리·시야 조건. 히스테리시스(5m / 7m)를 적용한 슈터의 판단을 그대로 쓴다. */
USTRUCT(meta = (DisplayName = "VX Shooter Range", Category = "VX|Shooter"))
struct VOXELCASTER_API FVXShooterRangeCondition : public FStateTreeAIConditionBase
{
	GENERATED_BODY()

	using FInstanceDataType = FVXShooterRangeConditionInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
