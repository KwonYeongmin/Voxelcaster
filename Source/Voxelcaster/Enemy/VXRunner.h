// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/VXEnemyBase.h"
#include "VXRunner.generated.h"

/**
 * 러너: 플레이어에게 곧장 돌진하고 접촉하면 피해 10. 체력 40, 이동 속도 5 m/s.
 * 같은 러너의 재접촉 피해는 0.8초 뒤부터 가능하다. 플레이어가 무적(대시)이면 피해를 주지 않는다. (DES-AI-001)
 */
UCLASS()
class VOXELCASTER_API AVXRunner : public AVXEnemyBase
{
	GENERATED_BODY()

public:
	AVXRunner();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void ApplyEnemyStats(const FVXEnemyRow& Row) override;

	/** 접촉 피해 */
	UPROPERTY(EditDefaultsOnly, Category = "Runner")
	float ContactDamage = 10.f;

	/** 같은 러너의 재접촉 피해 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Runner")
	float ContactCooldown = 0.8f;

	/** 접촉으로 보는 XY 거리 (cm). 두 캡슐 반경 합 + 여유 */
	UPROPERTY(EditDefaultsOnly, Category = "Runner")
	float ContactRange = 110.f;

private:
	float LastContactTime = -1000.f;
};
