// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VXGameplayEffects.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GAS/VXAttributeSet.h"

namespace VXEffects
{
	UGameplayEffect* MakeTagDurationEffect(UObject* Outer, FName Name, const FGameplayTagContainer& GrantedTags, float Duration)
	{
		UGameplayEffect* GE = NewObject<UGameplayEffect>(Outer, Name);
		GE->DurationPolicy = EGameplayEffectDurationType::HasDuration;
		GE->DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Duration));

		UTargetTagsGameplayEffectComponent& TagsComponent = GE->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
		FInheritedTagContainer TagChanges;
		TagChanges.Added = GrantedTags;
		TagsComponent.SetAndApplyTargetTagChanges(TagChanges);
		return GE;
	}

	UGameplayEffect* MakeDamageEffect(UObject* Outer, float Damage)
	{
		UGameplayEffect* GE = NewObject<UGameplayEffect>(Outer, TEXT("GE_Damage_Runtime"));
		GE->DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = UVXAttributeSet::GetIncomingDamageAttribute();
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Damage));
		GE->Modifiers.Add(Modifier);
		return GE;
	}

	void ApplyDamage(UAbilitySystemComponent* Target, float Damage, UAbilitySystemComponent* Instigator)
	{
		if (nullptr == Target || Damage <= 0.f)
		{
			return;
		}

		UAbilitySystemComponent* Source = Instigator ? Instigator : Target;
		UGameplayEffect* GE = MakeDamageEffect(GetTransientPackage(), Damage);
		Source->ApplyGameplayEffectToTarget(GE, Target, 1.f, Source->MakeEffectContext());
	}

	void ApplyHeal(UAbilitySystemComponent* Target, float Amount)
	{
		if (nullptr == Target || Amount <= 0.f)
		{
			return;
		}

		UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), TEXT("GE_Heal_Runtime"));
		GE->DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = UVXAttributeSet::GetHealthAttribute();
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Amount));
		GE->Modifiers.Add(Modifier);

		Target->ApplyGameplayEffectToSelf(GE, 1.f, Target->MakeEffectContext());
	}
}
