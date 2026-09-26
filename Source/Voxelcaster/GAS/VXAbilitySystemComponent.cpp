// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VXAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Voxelcaster.h"
#include "AbilitySystemInterface.h"
#include "GAS/VXGameplayEffects.h"

FGameplayAbilitySpecHandle UVXAbilitySystemComponent::GiveAbilityWithInput(TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& InputTag, int32 Level)
{
	if (nullptr == AbilityClass)
	{
		return FGameplayAbilitySpecHandle();
	}

	FGameplayAbilitySpec Spec(AbilityClass, Level);
	if (InputTag.IsValid())
	{
		Spec.GetDynamicSpecSourceTags().AddTag(InputTag);
	}
	return GiveAbility(Spec);
}

void UVXAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (false == InputTag.IsValid())
	{
		return;
	}

	HeldInputTags.AddTag(InputTag);

	int32 MatchCount = 0;
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			++MatchCount;
			AbilitySpecInputPressed(Spec);
			if (false == Spec.IsActive())
			{
				const bool bActivated = TryActivateAbility(Spec.Handle);
				UE_LOG(LogVX, Log, TEXT("Input %s pressed -> %s TryActivate=%s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability), bActivated ? TEXT("true") : TEXT("false"));
			}
		}
	}

	if (MatchCount == 0)
	{
		UE_LOG(LogVX, Warning, TEXT("Input %s pressed but no granted ability matches (granted: %d)"), *InputTag.ToString(), ActivatableAbilities.Items.Num());
	}
}

void UVXAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (false == InputTag.IsValid())
	{
		return;
	}

	HeldInputTags.RemoveTag(InputTag);

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(Spec);
		}
	}
}

void UVXAbilitySystemComponent::ProcessHeldInputs()
{
	if (HeldInputTags.IsEmpty())
	{
		return;
	}

	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (false == Spec.IsActive() && Spec.GetDynamicSpecSourceTags().HasAnyExact(HeldInputTags))
		{
			TryActivateAbility(Spec.Handle);
		}
	}
}

void UVXAbilitySystemComponent::ApplySkillHit(const FVXHitContext& Context)
{
	AActor* TargetActor = Context.Target.Get();
	if (nullptr == TargetActor)
	{
		return;
	}

	if (const IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(TargetActor))
	{
		VXEffects::ApplyDamage(TargetInterface->GetAbilitySystemComponent(), Context.Damage, this);
	}

	OnSkillHit.Broadcast(Context);
}
