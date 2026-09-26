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
};
