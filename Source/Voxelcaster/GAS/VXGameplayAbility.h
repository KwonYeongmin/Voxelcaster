// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "VXGameplayAbility.generated.h"

struct FVXSkillRow;

/**
 * 프로젝트 공용 어빌리티 베이스.
 * - 사망 후에는 활성화되지 않는다.
 * - bBlockedWhileDashing이 true면 대시 중 활성화되지 않는다 (스킬용).
 * - 쿨다운: CooldownDuration(초)과 GetCooldownTag()으로 쿨다운 GE를 코드로 만든다.
 * - 태그는 CDO 생성자에서 설정하지 않고 런타임 검사로 처리한다. (네이티브 태그 초기화 순서 문제 방지)
 */
UCLASS(Abstract)
class VOXELCASTER_API UVXGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UVXGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual UGameplayEffect* GetCooldownGameplayEffect() const override;
	virtual const FGameplayTagContainer* GetCooldownTags() const override;

	/**
	 * 코드로 만든 쿨다운 GE를 그대로 적용한다.
	 * 기본 구현은 GE의 클래스(CDO)로 스펙을 만들기 때문에, 런타임에 만든 GE 객체의 설정(시간·태그)이 사라진다.
	 */
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** 부여될 때 DT_Skills의 SkillRowName 행으로 수치를 덮어쓴다 */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/** 현재 적용되는 쿨다운(초). 가속 모디파이어 등이 재정의한다. */
	virtual float GetEffectiveCooldown() const { return CooldownDuration; }

protected:
	/** 런타임에 만든 GE 객체를 소유자에게 적용한다. (ApplyGameplayEffectToOwner는 GE 클래스의 CDO를 쓰므로 쓰지 않는다) */
	void ApplyRuntimeEffectToOwner(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const UGameplayEffect* Effect) const;

	/** 이 어빌리티의 쿨다운 태그. 없으면 쿨다운을 쓰지 않는다. */
	virtual FGameplayTag GetCooldownTag() const { return FGameplayTag(); }

	/** DT_Skills 행 수치를 적용한다. 하위 클래스는 자기 수치를 추가로 적용한다 (0 이하 값은 무시) */
	virtual void ApplySkillRow(const FVXSkillRow& Row);

	/** DT_Skills 행 이름. 비어 있으면 테이블을 쓰지 않는다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Data")
	FName SkillRowName;

	/** 기본 쿨다운 (초). DT_Skills의 Cooldown이 있으면 그 값을 쓴다 */
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
