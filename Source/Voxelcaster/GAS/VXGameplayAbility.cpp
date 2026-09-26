// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VXGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/VXGameplayEffects.h"
#include "GAS/VXGameplayTags.h"
#include "Data/VXSkillData.h"

UVXGameplayAbility::UVXGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UVXGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 액터별 인스턴스라 여기서 바꾼 값은 이 캐릭터에게만 적용된다.
	if (false == SkillRowName.IsNone())
	{
		if (const FVXSkillRow* Row = VXSkillData::Find(SkillRowName))
		{
			ApplySkillRow(*Row);
		}
	}
}

void UVXGameplayAbility::ApplySkillRow(const FVXSkillRow& Row)
{
	if (Row.Cooldown > 0.f)
	{
		CooldownDuration = Row.Cooldown;
	}
}

bool UVXGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (nullptr == ASC)
	{
		return false;
	}

	if (ASC->HasMatchingGameplayTag(VXTags::State_Dead))
	{
		return false;
	}
	if (bBlockedWhileDashing && ASC->HasMatchingGameplayTag(VXTags::State_Dashing))
	{
		return false;
	}
	return true;
}

UGameplayEffect* UVXGameplayAbility::GetCooldownGameplayEffect() const
{
	const FGameplayTag CooldownTag = GetCooldownTag();
	const float Duration = GetEffectiveCooldown();
	if (false == CooldownTag.IsValid() || Duration <= 0.f)
	{
		return nullptr;
	}

	// 쿨다운 시간이 바뀌면(가속 등) GE를 다시 만든다.
	if (nullptr == CooldownEffect || false == FMath::IsNearlyEqual(CachedCooldownDuration, Duration))
	{
		FGameplayTagContainer Tags;
		Tags.AddTag(CooldownTag);
		CooldownEffect = VXEffects::MakeTagDurationEffect(const_cast<UVXGameplayAbility*>(this), TEXT("GE_Cooldown"), Tags, Duration);
		CachedCooldownDuration = Duration;
	}
	return CooldownEffect;
}

void UVXGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	ApplyRuntimeEffectToOwner(Handle, ActorInfo, GetCooldownGameplayEffect());
}

void UVXGameplayAbility::ApplyRuntimeEffectToOwner(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const UGameplayEffect* Effect) const
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (nullptr == ASC || nullptr == Effect)
	{
		return;
	}

	ASC->ApplyGameplayEffectToSelf(Effect, GetAbilityLevel(Handle, ActorInfo), MakeEffectContext(Handle, ActorInfo));
}

const FGameplayTagContainer* UVXGameplayAbility::GetCooldownTags() const
{
	const FGameplayTag CooldownTag = GetCooldownTag();
	if (CooldownTagContainer.IsEmpty() && CooldownTag.IsValid())
	{
		CooldownTagContainer.AddTag(CooldownTag);
	}
	return &CooldownTagContainer;
}
