// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "VoxelGameplayAbility.generated.h"

/**
 * 프로젝트 공용 어빌리티 베이스.
 * - 사망 후에는 활성화되지 않는다.
 * - bBlockedWhileDashing이 true면 대시 중 활성화되지 않는다 (스킬용).
 * - 쿨다운: CooldownDuration(초)과 GetCooldownTag()으로 쿨다운 GE를 코드로 만든다.
 * - 태그는 CDO 생성자에서 설정하지 않고 런타임 검사로 처리한다. (네이티브 태그 초기화 순서 문제 방지)
 */
UCLASS(Abstract)
class VOXELCASTER_API UVoxelGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UVoxelGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/** 현재 적용되는 쿨다운(초). 가속 모디파이어 등이 재정의한다. */
	virtual float GetEffectiveCooldown() const { return CooldownDuration; }

protected:
	/** 이 어빌리티의 쿨다운 태그. 없으면 쿨다운을 쓰지 않는다. */
	virtual FGameplayTag GetCooldownTag() const { return FGameplayTag(); }

	/** 기본 쿨다운 (초). 수치 원본은 design 문서, 추후 데이터 테이블로 이동 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Cooldown")
	float CooldownDuration = 0.f;

	/** 대시 중에는 사용할 수 없는 어빌리티 (스킬). 대시 자체는 false. */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel")
	bool bBlockedWhileDashing = false;

private:
	UPROPERTY(Transient)
	mutable TObjectPtr<UGameplayEffect> CooldownEffect;

	mutable float CachedCooldownDuration = -1.f;
	mutable FGameplayTagContainer CooldownTagContainer;
};
