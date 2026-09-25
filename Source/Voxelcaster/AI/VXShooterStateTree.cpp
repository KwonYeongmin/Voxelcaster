// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/VXShooterStateTree.h"
#include "AIController.h"
#include "Enemy/VoxelShooter.h"
#include "StateTreeExecutionContext.h"

namespace
{
	AVXShooter* GetShooter(const AAIController* Controller)
	{
		return nullptr != Controller ? Cast<AVXShooter>(Controller->GetPawn()) : nullptr;
	}
}

EStateTreeRunStatus FVXShooterMoveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AVXShooter* Shooter = GetShooter(InstanceData.AIController);
	if (nullptr == Shooter)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (false == Shooter->IsDead())
	{
		Shooter->MoveRelativeToTarget(EVXShooterMoveMode::Retreat == InstanceData.Mode);
	}
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FVXShooterFireTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	AVXShooter* Shooter = GetShooter(InstanceData.AIController);
	if (nullptr == Shooter)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (false == Shooter->IsDead())
	{
		Shooter->TickFiring(DeltaTime);
	}
	return EStateTreeRunStatus::Running;
}

void FVXShooterFireTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (AVXShooter* Shooter = GetShooter(InstanceData.AIController))
	{
		Shooter->CancelTelegraph();
	}
}

bool FVXShooterRangeCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const AVXShooter* Shooter = GetShooter(InstanceData.AIController);
	if (nullptr == Shooter)
	{
		return false;
	}

	switch (InstanceData.Check)
	{
	case EVXShooterRangeCheck::CanHoldFire:
		return Shooter->CanHoldFire();
	case EVXShooterRangeCheck::ShouldApproach:
		return Shooter->ShouldApproach();
	case EVXShooterRangeCheck::ShouldRetreat:
		return Shooter->ShouldRetreat();
	case EVXShooterRangeCheck::RetreatDone:
		return Shooter->IsRetreatDone();
	}
	return false;
}
