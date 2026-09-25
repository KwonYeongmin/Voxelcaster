// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "VoxelHitContext.generated.h"

/** 스킬이 적 한 명을 명중했을 때의 정보. 모디파이어 훅(OnHit)이 이 정보를 받는다. (DES-MOD-001) */
USTRUCT(BlueprintType)
struct VOXELCASTER_API FVoxelHitContext
{
	GENERATED_BODY()

	/** 명중을 낸 스킬의 쿨다운 태그 등 스킬 식별용 태그 (예: Cooldown.MagicBolt) */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Source;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<AActor> Target;

	/** 명중 지점 (투사체는 닿은 지점, 범위·근접은 대상 위치) */
	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	/** 진행 방향 (투사체 방향, 범위 스킬은 시전자 → 대상 방향) */
	UPROPERTY(BlueprintReadWrite)
	FVector Direction = FVector::ForwardVector;

	/** 이 명중이 준 피해량 */
	UPROPERTY(BlueprintReadWrite)
	float Damage = 0.f;

	/** 파생 효과(분열 투사체, 폭발, 연쇄 번개)의 명중이면 true. 다른 모디파이어를 다시 발동하지 않는다. */
	UPROPERTY(BlueprintReadWrite)
	bool bIsDerived = false;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FVoxelSkillHitSignature, const FVoxelHitContext& /*Context*/);
