// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_RewardCard.h"

void UVX_VM_RewardCard::SetSkillName(const FText& InValue)
{
	VX_VM_SET_TEXT(SkillName, InValue);
}

void UVX_VM_RewardCard::SetModifierName(const FText& InValue)
{
	VX_VM_SET_TEXT(ModifierName, InValue);
}

void UVX_VM_RewardCard::SetLevelText(const FText& InValue)
{
	VX_VM_SET_TEXT(LevelText, InValue);
}

void UVX_VM_RewardCard::SetDescription(const FText& InValue)
{
	VX_VM_SET_TEXT(Description, InValue);
}

void UVX_VM_RewardCard::SetTagText(const FText& InValue)
{
	VX_VM_SET_TEXT(TagText, InValue);
}

void UVX_VM_RewardCard::SetbUpgrade(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bUpgrade, InValue);
}

void UVX_VM_RewardCard::SetbHighlighted(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHighlighted, InValue);
}
