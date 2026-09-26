// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXRewardCardViewModel.h"

void UVXRewardCardViewModel::SetSkillName(const FText& InValue)
{
	if (false == SkillName.EqualTo(InValue))
	{
		SkillName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SkillName);
	}
}

void UVXRewardCardViewModel::SetModifierName(const FText& InValue)
{
	if (false == ModifierName.EqualTo(InValue))
	{
		ModifierName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ModifierName);
	}
}

void UVXRewardCardViewModel::SetLevelText(const FText& InValue)
{
	if (false == LevelText.EqualTo(InValue))
	{
		LevelText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(LevelText);
	}
}

void UVXRewardCardViewModel::SetDescription(const FText& InValue)
{
	if (false == Description.EqualTo(InValue))
	{
		Description = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Description);
	}
}

void UVXRewardCardViewModel::SetTagText(const FText& InValue)
{
	if (false == TagText.EqualTo(InValue))
	{
		TagText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TagText);
	}
}

void UVXRewardCardViewModel::SetbUpgrade(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bUpgrade, InValue);
}

void UVXRewardCardViewModel::SetbHighlighted(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHighlighted, InValue);
}
