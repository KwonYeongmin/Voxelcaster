// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/VXShooter.h"
#include "VXElite.generated.h"

class AVXRunner;

/**
 * 엘리트 (슈터 강화): 웨이브 5에 1마리. (DES-ENEMY-001, DES-AI-001)
 * - 체력 400, 이동 속도 2 m/s, 유지 거리 7m
 * - 2초마다 플레이어 방향 중심 ±20°의 3방향 투사체 (각 피해 8)
 * - 체력이 50% 이하가 되면 0.8초 정지한 뒤 주변 2m에 러너 4마리를 소환한다. (1회)
 * - 소환된 러너는 웨이브 클리어 조건에 포함되고, 엘리트가 죽어도 남는다.
 */
UCLASS()
class VOXELCASTER_API AVXElite : public AVXShooter
{
	GENERATED_BODY()

public:
	AVXElite();

	virtual void Tick(float DeltaSeconds) override;

	bool IsSummoning() const { return bSummoning; }

protected:
	virtual void BeginPlay() override;
	virtual void FireAt(const AVXCharacterBase* Target) override;
	virtual void HandleDeath() override;

	/** 3방향 사격의 좌우 각도 (도) */
	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	float SpreadAngle = 20.f;

	/** 소환이 시작되는 체력 비율 */
	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	float SummonHealthRatio = 0.5f;

	/** 소환 시전 시간 (초). 이 동안 정지한다. */
	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	float SummonCastTime = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	int32 SummonCount = 4;

	/** 소환 위치 반경 (cm). 2m */
	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	float SummonRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	FLinearColor SummonColor = FLinearColor(1.f, 1.f, 1.f);

	/** 소환할 러너 클래스. 비워 두면 BP_VXRunner (없으면 C++ 러너) */
	UPROPERTY(EditDefaultsOnly, Category = "Elite")
	TSubclassOf<AVXRunner> MinionClass;

private:
	void HandleHealthChanged(float Current, float Max);
	void StartSummon();
	void FinishSummon();

	bool bHasSummoned = false;
	bool bSummoning = false;
	FTimerHandle SummonTimer;
};
