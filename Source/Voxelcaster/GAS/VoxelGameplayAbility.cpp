// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VoxelGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/VoxelGameplayEffects.h"
#include "GAS/VoxelGameplayTags.h"

UVoxelGameplayAbility::UVoxelGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

bool UVoxelGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	if (ASC->HasMatchingGameplayTag(VoxelTags::State_Dead))
	{
		return false;
	}
	if (bBlockedWhileDashing && ASC->HasMatchingGameplayTag(VoxelTags::State_Dashing))
	{
		return false;
	}
	return true;
}

UGameplayEffect* UVoxelGameplayAbility::GetCooldownGameplayEffect() const
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
		CooldownEffect = VoxelEffects::MakeTagDurationEffect(const_cast<UVoxelGameplayAbility*>(this), TEXT("GE_Cooldown"), Tags, Duration);
		CachedCooldownDuration = Duration;
	}
	return CooldownEffect;
}

void UVoxelGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	ApplyRuntimeEffectToOwner(Handle, ActorInfo, GetCooldownGameplayEffect());
}

void UVoxelGameplayAbility::ApplyRuntimeEffectToOwner(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const UGameplayEffect* Effect) const
{
	UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (nullptr == ASC || nullptr == Effect)
	{
		return;
	}

	ASC->ApplyGameplayEffectToSelf(Effect, GetAbilityLevel(Handle, ActorInfo), MakeEffectContext(Handle, ActorInfo));
}

const FGameplayTagContainer* UVoxelGameplayAbility::GetCooldownTags() const
{
	const FGameplayTag CooldownTag = GetCooldownTag();
	if (CooldownTagContainer.IsEmpty() && CooldownTag.IsValid())
	{
		CooldownTagContainer.AddTag(CooldownTag);
	}
	return &CooldownTagContainer;
}
