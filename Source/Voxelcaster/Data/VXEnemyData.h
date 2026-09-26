// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VXEnemyData.generated.h"

/**
 * 적 능력치 한 줄 (DES-DATA-001의 FEnemyRow). DT_Enemies의 행 구조체다.
 * 행 이름이 적 종류다: Runner, Shooter, Elite.
 * 테이블이 없거나 행이 없으면 적 클래스의 코드 기본값을 쓴다.
 */
USTRUCT(BlueprintType)
struct FVXEnemyRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	float MaxHealth = 40.f;

	/** 이동 속도 (cm/s). 5 m/s = 500 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float MoveSpeed = 500.f;

	/** 공격력: 러너는 접촉 피해, 슈터·엘리트는 투사체 한 발의 피해 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float AttackDamage = 10.f;

	/** 공격 간격 (초): 러너는 재접촉 간격, 슈터·엘리트는 발사 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.05", Units = "s"))
	float AttackInterval = 1.f;

	/** 투사체 속도 (cm/s). 0이면 코드 기본값. 러너는 쓰지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float ProjectileSpeed = 0.f;

	// ---- 군집 이동 (Boids). 음수면 코드 기본값 ----

	/** 이웃으로 보는 거리 (cm): 정렬·결합 계산 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock")
	float FlockRadius = -1.f;

	/** 분리: 가까운 적(종류 무관)에게서 멀어지는 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock")
	float SeparationWeight = -1.f;

	/** 정렬: 같은 종류 이웃의 평균 진행 방향을 따르는 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock")
	float AlignmentWeight = -1.f;

	/** 결합: 같은 종류 이웃의 평균 위치로 모이는 힘 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flock")
	float CohesionWeight = -1.f;
};
