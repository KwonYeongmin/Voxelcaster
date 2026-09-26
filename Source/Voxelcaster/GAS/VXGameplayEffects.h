// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/** 코드로 조립하는 간단한 GameplayEffect 헬퍼. 에셋 없이 수치를 코드/데이터에서 만든다. */
namespace VoxelEffects
{
	/** 지정한 시간 동안 태그를 부여하는 GE (무적, 쿨다운 등) */
	VOXELCASTER_API UGameplayEffect* MakeTagDurationEffect(UObject* Outer, FName Name, const FGameplayTagContainer& GrantedTags, float Duration);

	/** IncomingDamage 메타 어트리뷰트에 피해를 주는 즉발 GE */
	VOXELCASTER_API UGameplayEffect* MakeDamageEffect(UObject* Outer, float Damage);

	/** 대상에게 피해를 준다. 무적 상태면 무시된다. */
	VOXELCASTER_API void ApplyDamage(UAbilitySystemComponent* Target, float Damage, UAbilitySystemComponent* Instigator = nullptr);

	/** Health를 회복시킨다 (최대치 초과 불가). */
	VOXELCASTER_API void ApplyHeal(UAbilitySystemComponent* Target, float Amount);
}
