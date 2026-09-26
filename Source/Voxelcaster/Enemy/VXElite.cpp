// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VXElite.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Enemy/VXRunner.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTree.h"
#include "TimerManager.h"

AVXElite::AVXElite()
{
	StatRowName = TEXT("Elite");
	KillHitStop = 0.15f;
	KillVibrationIntensity = 1.f;
	KillVibrationDuration = 0.3f;
	DefaultMaxHealth = 400.f;
	DefaultMoveSpeed = 200.f;
	BodyColor = FLinearColor(0.9f, 0.1f, 0.45f);
	BodyScale = FVector(1.1f, 1.1f, 2.4f);

	// 슈터 행동을 그대로 쓰되 유지 거리 7m (6~8m 구간), 2초 간격
	HoldMinRange = 600.f;
	HoldMaxRange = 800.f;
	FireInterval = 2.f;
	ProjectileDamage = 8.f;

	// 엘리트는 아직 StateTree로 옮기지 않았다. 슈터의 StateTree를 물려받지 않고 기본 행동을 쓴다.
	StateTreeAsset.Reset();

	GetCapsuleComponent()->InitCapsuleSize(70.f, 120.f);
}

void AVXElite::BeginPlay()
{
	Super::BeginPlay();

	OnHealthChanged.AddUObject(this, &AVXElite::HandleHealthChanged);
}

void AVXElite::Tick(float DeltaSeconds)
{
	// 소환 시전 중에는 정지하고 이동·사격을 하지 않는다.
	if (bSummoning)
	{
		return;
	}

	Super::Tick(DeltaSeconds);
}

void AVXElite::FireAt(const AVXCharacterBase* Target)
{
	// 3방향: 플레이어 방향을 중심으로 좌·중·우
	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	SpawnProjectile(Direction, -SpreadAngle);
	SpawnProjectile(Direction, 0.f);
	SpawnProjectile(Direction, SpreadAngle);
}

void AVXElite::HandleHealthChanged(float Current, float Max)
{
	if (false == bHasSummoned && false == IsDead() && Current > 0.f && Current <= Max * SummonHealthRatio)
	{
		StartSummon();
	}
}

void AVXElite::StartSummon()
{
	bHasSummoned = true; // 한 번만
	bSummoning = true;

	GetCharacterMovement()->StopMovementImmediately();
	SetBodyColor(SummonColor);

	GetWorldTimerManager().SetTimer(SummonTimer, this, &AVXElite::FinishSummon, SummonCastTime, false);
}

void AVXElite::FinishSummon()
{
	bSummoning = false;
	SetBodyColor(BodyColor);

	if (IsDead())
	{
		return;
	}

	// 러너 4마리를 엘리트 주변 2m에 균등하게 생성한다. 바닥 높이는 캡슐 높이 차이만큼 보정한다.
	const float EliteHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float RunnerHalfHeight = AVXRunner::StaticClass()->GetDefaultObject<AVXRunner>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float FeetZ = GetActorLocation().Z - EliteHalfHeight;

	for (int32 i = 0; i < SummonCount; ++i)
	{
		const float Angle = 360.f * i / FMath::Max(SummonCount, 1);
		const FVector Offset = FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * SummonRadius;
		const FVector Location(GetActorLocation().X + Offset.X, GetActorLocation().Y + Offset.Y, FeetZ + RunnerHalfHeight + 2.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AVXRunner* Runner = GetWorld()->SpawnActor<AVXRunner>(AVXRunner::StaticClass(), Location, FRotator::ZeroRotator, Params))
		{
			OnMinionSpawned.Broadcast(Runner);
		}
	}
}

void AVXElite::HandleDeath()
{
	// 소환 시전 중에 죽으면 소환은 취소된다.
	bSummoning = false;
	GetWorldTimerManager().ClearTimer(SummonTimer);

	Super::HandleDeath();
}
