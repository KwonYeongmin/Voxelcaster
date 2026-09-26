// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_RewardCard.h"

void UVX_VM_RewardCard::SetSkillName(const FText& InValue)
{
	if (false == SkillName.EqualTo(InValue))
	{
		SkillName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SkillName);
	}
}

void UVX_VM_RewardCard::SetModifierName(const FText& InValue)
{
	if (false == ModifierName.EqualTo(InValue))
	{
		ModifierName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ModifierName);
	}
}

void UVX_VM_RewardCard::SetLevelText(const FText& InValue)
{
	if (false == LevelText.EqualTo(InValue))
	{
		LevelText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(LevelText);
	}
}

void UVX_VM_RewardCard::SetDescription(const FText& InValue)
{
	if (false == Description.EqualTo(InValue))
	{
		Description = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Description);
	}
}

void UVX_VM_RewardCard::SetTagText(const FText& InValue)
{
	if (false == TagText.EqualTo(InValue))
	{
		TagText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TagText);
	}
}

void UVX_VM_RewardCard::SetbUpgrade(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bUpgrade, InValue);
}

void UVX_VM_RewardCard::SetbHighlighted(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHighlighted, InValue);
}
