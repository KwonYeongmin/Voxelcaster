// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXSkillSlotViewModel.h"

void UVXSkillSlotViewModel::SetSkillName(const FText& InValue)
{
	if (false == SkillName.EqualTo(InValue))
	{
		SkillName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SkillName);
	}
}

void UVXSkillSlotViewModel::SetKeyText(const FText& InValue)
{
	if (false == KeyText.EqualTo(InValue))
	{
		KeyText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(KeyText);
	}
}

void UVXSkillSlotViewModel::SetCooldownPercent(float InValue)
{
	if (false == FMath::IsNearlyEqual(CooldownPercent, InValue, 0.001f))
	{
		CooldownPercent = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownPercent);
	}
}

void UVXSkillSlotViewModel::SetCooldownText(const FText& InValue)
{
	if (false == CooldownText.EqualTo(InValue))
	{
		CooldownText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownText);
	}
}

void UVXSkillSlotViewModel::SetbReady(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bReady, InValue);
}

void UVXSkillSlotViewModel::SetModifiersText(const FText& InValue)
{
	if (false == ModifiersText.EqualTo(InValue))
	{
		ModifiersText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ModifiersText);
	}
}
