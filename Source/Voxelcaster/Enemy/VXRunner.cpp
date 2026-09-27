// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VXRunner.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "Data/VXEnemyData.h"
#include "GAS/VXGameplayEffects.h"

AVXRunner::AVXRunner()
{
	PrimaryActorTick.bCanEverTick = true;

	StatRowName = TEXT("Runner");
	DefaultMaxHealth = 40.f;
	DefaultMoveSpeed = 500.f;
	BodyColor = FLinearColor(0.9f, 0.15f, 0.1f);

	// 러너는 떼 지어 몰려온다 (추적보다 약하게 둬야 플레이어를 놓치지 않는다)
	AlignmentWeight = 0.35f;
	CohesionWeight = 0.25f;
}

void AVXRunner::ApplyEnemyStats(const FVXEnemyRow& Row)
{
	Super::ApplyEnemyStats(Row);
	ContactDamage = Row.AttackDamage;
	ContactCooldown = Row.AttackInterval;
}

void AVXRunner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDead())
	{
		return;
	}

	AVXCharacterBase* Player = FindLivePlayer();
	if (nullptr == Player)
	{
		return;
	}

	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	// 벽에 가려 있으면 내비메시 경로를 따라 돌아온다
	const FVector Direction = GetPathDirectionTo(Player);

	// Chase: 플레이어를 향해 최대 속도로 이동한다.
	AddMovementInput(Direction, 1.f);
	if (false == Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}

	// Contact: 접촉 시 피해 (재접촉 쿨다운). 대시 무적 중에는 GE가 무시하므로 피해가 들어가지 않는다.
	const float Now = GetWorld()->GetTimeSeconds();
	if (ToPlayer.Size2D() <= ContactRange && Now - LastContactTime >= ContactCooldown)
	{
		LastContactTime = Now;
		VXEffects::ApplyDamage(Player->GetAbilitySystemComponent(), GetAdjustedAttackDamage(ContactDamage), GetVoxelAbilitySystemComponent());
	}
}
