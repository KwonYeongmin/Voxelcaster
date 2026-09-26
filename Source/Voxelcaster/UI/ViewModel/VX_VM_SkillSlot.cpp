// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_SkillSlot.h"
#include "Engine/Texture2D.h"

void UVX_VM_SkillSlot::SetSkillName(const FText& InValue)
{
	VX_VM_SET_TEXT(SkillName, InValue);
}

void UVX_VM_SkillSlot::SetKeyText(const FText& InValue)
{
	VX_VM_SET_TEXT(KeyText, InValue);
}

void UVX_VM_SkillSlot::SetKeyIcon(UTexture2D* InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(KeyIcon, InValue);
}

void UVX_VM_SkillSlot::SetbHasKeyIcon(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bHasKeyIcon, InValue);
}

void UVX_VM_SkillSlot::SetCooldownPercent(float InValue)
{
	VX_VM_SET_FLOAT(CooldownPercent, InValue);
}

void UVX_VM_SkillSlot::SetCooldownText(const FText& InValue)
{
	VX_VM_SET_TEXT(CooldownText, InValue);
}

void UVX_VM_SkillSlot::SetbReady(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bReady, InValue);
}

void UVX_VM_SkillSlot::SetModifiersText(const FText& InValue)
{
	VX_VM_SET_TEXT(ModifiersText, InValue);
}
