// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/VXGameplayAbility.h"
#include "VXSkillAbility.generated.h"

class AVXCharacterBase;

/**
 * 스킬 3종의 공통 베이스 (DES-SKILL-001).
 * 활성화되면 쿨다운을 시작하고 ExecuteSkill을 호출한 뒤 즉시 종료한다. 대시 중에는 사용할 수 없다.
 * 피해는 모두 ApplyHit -> UVXAbilitySystemComponent::ApplySkillHit를 거친다. (모디파이어 훅)
 */
UCLASS(Abstract)
class VOXELCASTER_API UVXSkillAbility : public UVXGameplayAbility
{
	GENERATED_BODY()

public:
	UVXSkillAbility();

	/** 가속 모디파이어를 반영한 쿨다운 */
	virtual float GetEffectiveCooldown() const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	/** 시전자의 모디파이어 컴포넌트 (없으면 nullptr) */
	class UVXModifierComponent* GetModifierComponent() const;

	/** 스킬 본체. 시전자를 받아 형태별 동작을 수행한다. */
	virtual void ExecuteSkill(AVXCharacterBase* Caster) PURE_VIRTUAL(UVXSkillAbility::ExecuteSkill, );

	/** 시전자와 적대적인 캐릭터(살아 있는)를 구 범위 안에서 모은다. */
	void GatherHostilesInRadius(const AVXCharacterBase* Caster, const FVector& Center, float Radius, TArray<AVXCharacterBase*>& OutTargets) const;

	/** 대상 한 명에게 스킬 명중을 적용한다. Damage 배율을 곱한다. */
	void ApplyHit(AVXCharacterBase* Caster, AVXCharacterBase* Target, const FVector& HitLocation, const FVector& Direction, float DamageScale = 1.f) const;

	/** 원본 피해 (design 수치, 추후 데이터 테이블로 이동) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Skill")
	float Damage = 0.f;
};
