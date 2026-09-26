// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Hud.h"

UVX_VM_Hud::UVX_VM_Hud()
{
	SkillSlot0 = CreateDefaultSubobject<UVX_VM_SkillSlot>(TEXT("SkillSlot0"));
	SkillSlot1 = CreateDefaultSubobject<UVX_VM_SkillSlot>(TEXT("SkillSlot1"));
	SkillSlot2 = CreateDefaultSubobject<UVX_VM_SkillSlot>(TEXT("SkillSlot2"));
	SkillSlot3 = CreateDefaultSubobject<UVX_VM_SkillSlot>(TEXT("SkillSlot3"));
}

void UVX_VM_Hud::SetHealthPercent(float InValue)
{
	VX_VM_SET_FLOAT(HealthPercent, InValue);
}

void UVX_VM_Hud::SetHealthText(const FText& InValue)
{
	VX_VM_SET_TEXT(HealthText, InValue);
}

void UVX_VM_Hud::SetbLowHealth(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bLowHealth, InValue);
}

void UVX_VM_Hud::SetWaveText(const FText& InValue)
{
	VX_VM_SET_TEXT(WaveText, InValue);
}

void UVX_VM_Hud::SetEnemiesText(const FText& InValue)
{
	VX_VM_SET_TEXT(EnemiesText, InValue);
}

void UVX_VM_Hud::SetBannerText(const FText& InValue)
{
	VX_VM_SET_TEXT(BannerText, InValue);
}

void UVX_VM_Hud::SetBannerSubText(const FText& InValue)
{
	VX_VM_SET_TEXT(BannerSubText, InValue);
}

void UVX_VM_Hud::SetBannerOpacity(float InValue)
{
	VX_VM_SET_FLOAT(BannerOpacity, InValue);
}
