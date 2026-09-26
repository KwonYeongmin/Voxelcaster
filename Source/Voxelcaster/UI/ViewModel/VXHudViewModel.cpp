// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXHudViewModel.h"

UVXHudViewModel::UVXHudViewModel()
{
	SkillSlot0 = CreateDefaultSubobject<UVXSkillSlotViewModel>(TEXT("SkillSlot0"));
	SkillSlot1 = CreateDefaultSubobject<UVXSkillSlotViewModel>(TEXT("SkillSlot1"));
	SkillSlot2 = CreateDefaultSubobject<UVXSkillSlotViewModel>(TEXT("SkillSlot2"));
	SkillSlot3 = CreateDefaultSubobject<UVXSkillSlotViewModel>(TEXT("SkillSlot3"));
}

void UVXHudViewModel::SetHealthPercent(float InValue)
{
	if (false == FMath::IsNearlyEqual(HealthPercent, InValue, 0.001f))
	{
		HealthPercent = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HealthPercent);
	}
}

void UVXHudViewModel::SetHealthText(const FText& InValue)
{
	if (false == HealthText.EqualTo(InValue))
	{
		HealthText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HealthText);
	}
}

void UVXHudViewModel::SetbLowHealth(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bLowHealth, InValue);
}

void UVXHudViewModel::SetWaveText(const FText& InValue)
{
	if (false == WaveText.EqualTo(InValue))
	{
		WaveText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(WaveText);
	}
}

void UVXHudViewModel::SetEnemiesText(const FText& InValue)
{
	if (false == EnemiesText.EqualTo(InValue))
	{
		EnemiesText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(EnemiesText);
	}
}
