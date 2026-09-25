// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VXWaveData.generated.h"

class AVXEnemyBase;

/**
 * 스폰 규칙 한 줄: 어떤 적을, 언제부터, 몇 초마다, 한 번에 몇 마리씩, 총 몇 마리 내보낼지.
 * 시간은 웨이브의 전투 시작(시작 연출이 끝난 시점) 기준이다.
 * 예) StartTime 0, Interval 5, CountPerSpawn 3, TotalCount 12 → 0·5·10·15초에 3마리씩
 */
USTRUCT(BlueprintType)
struct FVXWaveSpawn
{
	GENERATED_BODY()

	/** 스폰할 적 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AVXEnemyBase> EnemyClass;

	/** 첫 스폰 시각 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", Units = "s"))
	float StartTime = 0.f;

	/** 스폰 주기 (초). 0이면 StartTime에 한꺼번에 나온다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", Units = "s"))
	float Interval = 5.f;

	/** 한 번에 나오는 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 CountPerSpawn = 1;

	/** 이 줄에서 나오는 총 수. 도달하면 멈춘다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 TotalCount = 1;
};

/**
 * 웨이브 데이터 테이블 행 (DES-DATA-001의 FWaveRow). DT_Waves의 행 구조체다.
 * 행 이름은 자유롭고, 진행 순서는 Index(1부터)로 정한다.
 * 스폰 규칙의 적이 모두 나오고 모두 처치되면 웨이브가 클리어된다.
 */
USTRUCT(BlueprintType)
struct FVXWaveRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 웨이브 번호 (1부터). 이 값의 오름차순으로 진행한다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVXWaveSpawn> Spawns;
};
