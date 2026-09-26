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
	if (false == FMath::IsNearlyEqual(HealthPercent, InValue, 0.001f))
	{
		HealthPercent = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HealthPercent);
	}
}

void UVX_VM_Hud::SetHealthText(const FText& InValue)
{
	if (false == HealthText.EqualTo(InValue))
	{
		HealthText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HealthText);
	}
}

void UVX_VM_Hud::SetbLowHealth(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bLowHealth, InValue);
}

void UVX_VM_Hud::SetWaveText(const FText& InValue)
{
	if (false == WaveText.EqualTo(InValue))
	{
		WaveText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(WaveText);
	}
}

void UVX_VM_Hud::SetEnemiesText(const FText& InValue)
{
	if (false == EnemiesText.EqualTo(InValue))
	{
		EnemiesText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EnemiesText);
	}
}
