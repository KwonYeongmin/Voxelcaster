// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VXShooter.h"
#include "Animation/AnimMontage.h"
#include "Combat/VXProjectile.h"
#include "Data/VXEnemyData.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "StateTree.h"

AVXShooter::AVXShooter()
{
	PrimaryActorTick.bCanEverTick = true;

	StatRowName = TEXT("Shooter");
	DefaultMaxHealth = 60.f;
	DefaultMoveSpeed = 250.f;
	BodyColor = FLinearColor(0.55f, 0.2f, 0.9f);

	// 에디터에서 만든 StateTree. 에셋이 없으면 Tick의 기본 행동으로 동작한다.
	StateTreeAsset = TSoftObjectPtr<UStateTree>(FSoftObjectPath(TEXT("/Game/Voxelcaster/AI/ST_Shooter.ST_Shooter")));
}

void AVXShooter::BeginPlay()
{
	Super::BeginPlay();

	// 여러 슈터가 동시에 쏘지 않도록 첫 발사 시점을 흩는다.
	FireTimer = FMath::FRandRange(0.f, FireInterval);
}

void AVXShooter::ApplyEnemyStats(const FVXEnemyRow& Row)
{
	Super::ApplyEnemyStats(Row);
	ProjectileDamage = Row.AttackDamage;
	FireInterval = Row.AttackInterval;
	if (Row.ProjectileSpeed > 0.f)
	{
		ProjectileSpeed = Row.ProjectileSpeed;
	}
}

void AVXShooter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsDead() || IsDrivenByStateTree())
	{
		return;
	}

	TickBuiltInBehavior(DeltaSeconds);
}

// ---------------------------------------------------------------------------
// 판단 (거리·시야)
// ---------------------------------------------------------------------------

float AVXShooter::GetDistanceToTarget() const
{
	const AVXCharacterBase* Player = FindLivePlayer();
	return nullptr != Player ? (Player->GetActorLocation() - GetActorLocation()).Size2D() : -1.f;
}

bool AVXShooter::HasLineOfSightTo(const AVXCharacterBase* Target) const
{
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VXShooterLOS), false, this);
	Params.AddIgnoredActor(Target);

	FHitResult Hit;
	return false == GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation(), Target->GetActorLocation(), ECC_Visibility, Params);
}

bool AVXShooter::CanHoldFire() const
{
	const AVXCharacterBase* Player = FindLivePlayer();
	return nullptr != Player && GetDistanceToTarget() <= HoldMaxRange && HasLineOfSightTo(Player);
}

bool AVXShooter::ShouldApproach() const
{
	const AVXCharacterBase* Player = FindLivePlayer();
	return nullptr != Player && (GetDistanceToTarget() > HoldMaxRange || false == HasLineOfSightTo(Player));
}

bool AVXShooter::ShouldRetreat() const
{
	const float Distance = GetDistanceToTarget();
	return Distance >= 0.f && Distance < HoldMinRange;
}

bool AVXShooter::IsRetreatDone() const
{
	return GetDistanceToTarget() >= HoldMinRange;
}

// ---------------------------------------------------------------------------
// 동작
// ---------------------------------------------------------------------------

void AVXShooter::FaceTarget()
{
	const AVXCharacterBase* Player = FindLivePlayer();
	if (nullptr == Player)
	{
		return;
	}

	const FVector Direction = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (false == Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void AVXShooter::MoveRelativeToTarget(bool bAway)
{
	const AVXCharacterBase* Player = FindLivePlayer();
	if (nullptr == Player)
	{
		return;
	}

	// 다가갈 때는 길찾기(벽을 돌아서), 물러날 때는 반대 방향으로 곧게
	const FVector Direction = bAway
		? -(Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D()
		: GetPathDirectionTo(Player);
	AddMovementInput(Direction, 1.f);
	FaceTarget();
}

void AVXShooter::TickFiring(float DeltaSeconds)
{
	const AVXCharacterBase* Player = FindLivePlayer();
	if (nullptr == Player)
	{
		CancelTelegraph();
		return;
	}

	FaceTarget();
	FireTimer -= DeltaSeconds;

	if (false == bTelegraphing && FireTimer <= TelegraphTime)
	{
		bTelegraphing = true;
		SetBodyColor(TelegraphColor);
	}

	if (FireTimer <= 0.f)
	{
		if (FireMontage && HasSkeletalMesh())
		{
			PlayAnimMontage(FireMontage);
		}
		FireAt(Player);
		FireTimer = FireInterval;
		CancelTelegraph();
	}
}

void AVXShooter::CancelTelegraph()
{
	if (bTelegraphing)
	{
		bTelegraphing = false;
		SetBodyColor(BodyColor);
	}
}

// ---------------------------------------------------------------------------
// StateTree가 없을 때의 기본 행동 (같은 판단·동작 함수를 쓴다)
// ---------------------------------------------------------------------------

void AVXShooter::TickBuiltInBehavior(float DeltaSeconds)
{
	if (nullptr == FindLivePlayer())
	{
		CancelTelegraph();
		return;
	}

	FaceTarget();

	// 상태 전환 (히스테리시스: 5m / 7m)
	switch (ShooterState)
	{
	case EVXShooterState::Approach:
		if (CanHoldFire())
		{
			ShooterState = EVXShooterState::Hold;
		}
		break;
	case EVXShooterState::Hold:
		if (ShouldRetreat())
		{
			ShooterState = EVXShooterState::Retreat;
		}
		else if (ShouldApproach())
		{
			ShooterState = EVXShooterState::Approach;
		}
		break;
	case EVXShooterState::Retreat:
		if (IsRetreatDone())
		{
			ShooterState = EVXShooterState::Hold;
		}
		break;
	}

	if (EVXShooterState::Approach == ShooterState)
	{
		MoveRelativeToTarget(false);
	}
	else if (EVXShooterState::Retreat == ShooterState)
	{
		MoveRelativeToTarget(true);
	}

	// 사격은 Hold일 때만 한다. 그 외 상태에서는 예고를 취소한다.
	if (EVXShooterState::Hold == ShooterState)
	{
		TickFiring(DeltaSeconds);
	}
	else
	{
		CancelTelegraph();
	}
}

// ---------------------------------------------------------------------------
// 사격
// ---------------------------------------------------------------------------

void AVXShooter::FireAt(const AVXCharacterBase* Target)
{
	// 발사 순간의 플레이어 위치를 향한다. (예측 사격 없음)
	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	SpawnProjectile(Direction, 0.f);
}

void AVXShooter::SpawnProjectile(const FVector& BaseDirection, float AngleOffset)
{
	const FVector Direction = BaseDirection.RotateAngleAxis(AngleOffset, FVector::UpVector);
	const FVector SpawnLocation = GetActorLocation() + Direction * 60.f;
	const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);

	AVXProjectile* Projectile = GetWorld()->SpawnActorDeferred<AVXProjectile>(
		AVXProjectile::StaticClass(), SpawnTransform, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		Projectile->Init(this, FGameplayTag(), GetAdjustedAttackDamage(ProjectileDamage), ProjectileSpeed, ProjectileMaxRange, FLinearColor(1.f, 0.35f, 0.1f));
		Projectile->FinishSpawning(SpawnTransform);
	}
}
