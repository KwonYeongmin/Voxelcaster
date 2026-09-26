// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VXSkillData.generated.h"

/**
 * 스킬 수치 한 줄. DT_Skills의 행 구조체다.
 * 행 이름: MagicBolt, Nova, BladeSweep, Dash
 * 스킬마다 쓰는 열만 읽는다. 0 이하인 값은 무시하고 코드 기본값을 쓴다.
 */
USTRUCT(BlueprintType)
struct FVXSkillRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 피해 (노바는 틱당 피해). 대시는 쓰지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float Damage = 0.f;

	/** 쿨다운 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "s"))
	float Cooldown = 0.f;

	/** 반경 (cm): 노바 장판, 블레이드 스윕 부채꼴 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float Radius = 0.f;

	/** 부채꼴 각도 (도): 블레이드 스윕 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "360"))
	float ArcAngle = 0.f;

	/** 투사체 속도 (cm/s): 매직 볼트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float ProjectileSpeed = 0.f;

	/** 최대 사거리 (cm): 매직 볼트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float MaxRange = 0.f;

	/** 지속 시간 (초): 노바 장판, 대시 이동 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "s"))
	float Duration = 0.f;

	/** 피해 간격 (초): 노바 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "s"))
	float TickInterval = 0.f;

	/** 이동 거리 (cm): 대시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DashDistance = 0.f;

	/** 무적 시간 (초): 대시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", Units = "s"))
	float InvincibleTime = 0.f;
};

namespace VXSkillData
{
	/** DT_Skills에서 행을 찾는다. 테이블이나 행이 없으면 nullptr (코드 기본값 사용) */
	VOXELCASTER_API const FVXSkillRow* Find(FName RowName);
}
