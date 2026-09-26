// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Modifier/VXModifierComponent.h"
#include "VXModifierData.generated.h"

/**
 * 모디파이어 수치 한 줄. DT_Modifiers의 행 구조체다.
 * 행 이름: Pierce, Split, Explode, Chain, Haste
 * 스택별 값 = BaseValue + PerStackValue × (스택 - 1)
 *   Pierce  관통 수          Split 분열 투사체 수
 *   Explode 폭발 반경 (cm)   Chain 전이 횟수
 *   Haste   쿨다운 감소 비율 (0.15 = 15%)
 * 테이블이나 행이 없으면 코드 기본값을 쓴다.
 */
USTRUCT(BlueprintType)
struct FVXModifierRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 1스택 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 0.f;

	/** 스택이 하나 늘 때마다 더하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerStackValue = 0.f;

	/** 원본 피해 대비 파생 피해 비율: Split, Explode, Chain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float DamageRatio = 0.f;

	/** 사거리 (cm): Split 투사체 사거리, Chain 전이 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float Range = 0.f;

	/** 투사체 속도 (cm/s): Split */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
	float Speed = 0.f;

	/** 스택별 값 */
	float GetValue(int32 Stack) const { return BaseValue + PerStackValue * FMath::Max(Stack - 1, 0); }

	/** 정수 값 (관통 수, 분열 수, 전이 횟수) */
	int32 GetCount(int32 Stack) const { return FMath::Max(FMath::RoundToInt(GetValue(Stack)), 0); }
};

namespace VXModifierData
{
	/** 모디파이어 수치. DT_Modifiers 행이 있으면 그 값, 없으면 코드 기본값 */
	VOXELCASTER_API const FVXModifierRow& Get(EVXModifierType Type);
}
