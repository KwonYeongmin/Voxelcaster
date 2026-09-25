// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VoxelAbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Voxelcaster.h"
#include "AbilitySystemInterface.h"
#include "GAS/VoxelGameplayEffects.h"

FGameplayAbilitySpecHandle UVoxelAbilitySystemComponent::GiveAbilityWithInput(TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& InputTag, int32 Level)
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

void UVoxelAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
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
				UE_LOG(LogVoxel, Log, TEXT("Input %s pressed -> %s TryActivate=%s"), *InputTag.ToString(), *GetNameSafe(Spec.Ability), bActivated ? TEXT("true") : TEXT("false"));
			}
		}
	}

	if (MatchCount == 0)
	{
		UE_LOG(LogVoxel, Warning, TEXT("Input %s pressed but no granted ability matches (granted: %d)"), *InputTag.ToString(), ActivatableAbilities.Items.Num());
	}
}

void UVoxelAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
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

void UVoxelAbilitySystemComponent::ProcessHeldInputs()
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

void UVoxelAbilitySystemComponent::ApplySkillHit(const FVoxelHitContext& Context)
{
	AActor* TargetActor = Context.Target.Get();
	if (nullptr == TargetActor)
	{
		return;
	}

	if (const IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(TargetActor))
	{
		VoxelEffects::ApplyDamage(TargetInterface->GetAbilitySystemComponent(), Context.Damage, this);
	}

	OnSkillHit.Broadcast(Context);
}
