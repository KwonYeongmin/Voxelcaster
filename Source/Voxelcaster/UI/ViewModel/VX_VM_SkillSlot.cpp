// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_SkillSlot.h"

void UVX_VM_SkillSlot::SetSkillName(const FText& InValue)
{
	if (false == SkillName.EqualTo(InValue))
	{
		SkillName = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(SkillName);
	}
}

void UVX_VM_SkillSlot::SetKeyText(const FText& InValue)
{
	if (false == KeyText.EqualTo(InValue))
	{
		KeyText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(KeyText);
	}
}

void UVX_VM_SkillSlot::SetCooldownPercent(float InValue)
{
	if (false == FMath::IsNearlyEqual(CooldownPercent, InValue, 0.001f))
	{
		CooldownPercent = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownPercent);
	}
}

void UVX_VM_SkillSlot::SetCooldownText(const FText& InValue)
{
	if (false == CooldownText.EqualTo(InValue))
	{
		CooldownText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CooldownText);
	}
}

void UVX_VM_SkillSlot::SetbReady(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bReady, InValue);
}

void UVX_VM_SkillSlot::SetModifiersText(const FText& InValue)
{
	if (false == ModifiersText.EqualTo(InValue))
	{
		ModifiersText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ModifiersText);
	}
}
