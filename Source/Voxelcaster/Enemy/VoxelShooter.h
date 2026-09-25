// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/VoxelEnemyBase.h"
#include "VoxelShooter.generated.h"

UENUM()
enum class EVXShooterState : uint8
{
	/** 플레이어와 멀다: 다가간다 */
	Approach,
	/** 적정 거리: 정지하고 사격한다 */
	Hold,
	/** 플레이어와 가깝다: 물러난다 */
	Retreat
};

/**
 * 슈터: 플레이어와 6m를 유지하며 1.5초마다 투사체를 쏜다. 체력 60, 이동 속도 2.5 m/s. (DES-ENEMY-001, DES-AI-001)
 * - 7m 밖이면 Approach, 5~7m이면 Hold(사격), 5m 안이면 Retreat. 경계에서 떨리지 않도록 ±1m 구간을 둔다.
 * - 발사 0.3초 전 몸이 밝게 빛나 회피 단서를 준다.
 * - 투사체는 발사 순간의 플레이어 위치를 향해 직선으로 날아간다. (예측 사격 없음)
 * - 사이에 벽이 있으면 쏘지 않고 다가간다.
 *
 * 행동 구동: StateTree(ST_Shooter)가 지정되어 있으면 StateTree가 상태 전환을 맡고,
 * 없으면 이 클래스의 Tick이 같은 함수들로 상태 머신을 돌린다. (StateTree 에셋을 만들기 전 대비)
 * 공개 함수(CanHoldFire 등)는 StateTree 노드(VXShooterStateTree)가 호출한다.
 */
UCLASS()
class VOXELCASTER_API AVXShooter : public AVXEnemyBase
{
	GENERATED_BODY()

public:
	AVXShooter();

	virtual void Tick(float DeltaSeconds) override;

	// ---- StateTree 노드가 쓰는 판단 (거리·시야) ----

	/** 사격 가능: 최대 유지 거리 안이고 시야가 트여 있다 */
	bool CanHoldFire() const;
	/** 접근 필요: 최대 유지 거리 밖이거나 시야가 막혔다 */
	bool ShouldApproach() const;
	/** 후퇴 필요: 최소 유지 거리 안이다 */
	bool ShouldRetreat() const;
	/** 후퇴 완료: 최소 유지 거리 이상이다 */
	bool IsRetreatDone() const;

	// ---- StateTree 노드가 쓰는 동작 ----

	/** 플레이어를 바라본다 */
	void FaceTarget();
	/** 플레이어를 향해(또는 반대로) 이동 입력을 넣는다. 플레이어를 바라보기도 한다. */
	void MoveRelativeToTarget(bool bAway);
	/** 사격 타이머를 진행한다: 예고 → 발사 → 재장전. Hold 상태에서 매 틱 호출한다. */
	void TickFiring(float DeltaSeconds);
	/** 발사 예고(몸 색)를 취소한다. Hold를 벗어날 때 호출한다. */
	void CancelTelegraph();

protected:
	virtual void BeginPlay() override;

	/** 이 거리보다 멀면 접근 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float HoldMaxRange = 700.f;

	/** 이 거리보다 가까우면 후퇴 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float HoldMinRange = 500.f;

	/** 발사 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float FireInterval = 1.5f;

	/** 발사 전 예고 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float TelegraphTime = 0.3f;

	/** 투사체 피해 */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float ProjectileDamage = 8.f;

	/** 투사체 속도 (cm/s). 8 m/s = 800 */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float ProjectileSpeed = 800.f;

	/** 투사체 최대 사거리 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	float ProjectileMaxRange = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Shooter")
	FLinearColor TelegraphColor = FLinearColor(1.f, 0.9f, 0.2f);

	/** 사격 본체. 엘리트가 3방향 사격으로 재정의한다. */
	virtual void FireAt(const AVXCharacterBase* Target);

	/** 플레이어를 향해 투사체 한 발을 쏜다. AngleOffset은 기준 방향에서 벌어진 각도(도) */
	void SpawnProjectile(const FVector& BaseDirection, float AngleOffset);

private:
	/** 플레이어까지의 XY 거리. 살아 있는 플레이어가 없으면 -1 */
	float GetDistanceToTarget() const;
	bool HasLineOfSightTo(const AVXCharacterBase* Target) const;

	/** StateTree가 없을 때 쓰는 기본 행동 */
	void TickBuiltInBehavior(float DeltaSeconds);

	EVXShooterState ShooterState = EVXShooterState::Approach;

	/** 다음 발사까지 남은 시간 */
	float FireTimer = 0.f;
	bool bTelegraphing = false;
};
