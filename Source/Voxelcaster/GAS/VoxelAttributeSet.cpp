// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/VoxelAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GAS/VoxelGameplayTags.h"

void UVoxelAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

bool UVoxelAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (false == Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// 무적 상태(대시 무적)와 사망 후에는 피해를 무시한다.
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute() && Data.EvaluatedData.Magnitude > 0.f)
	{
		const UAbilitySystemComponent& ASC = Data.Target;
		if (ASC.HasMatchingGameplayTag(VoxelTags::State_Invincible) || ASC.HasMatchingGameplayTag(VoxelTags::State_Dead))
		{
			return false;
		}
	}
	return true;
}

void UVoxelAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (Damage > 0.f)
		{
			SetHealth(FMath::Clamp(GetHealth() - Damage, 0.f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}
