// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Result.h"

void UVX_VM_Result::SetTitleText(const FText& InValue)
{
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVX_VM_Result::SetbVictory(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bVictory, InValue);
}

void UVX_VM_Result::SetStatsText(const FText& InValue)
{
	if (false == StatsText.EqualTo(InValue))
	{
		StatsText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(StatsText);
	}
}

void UVX_VM_Result::SetBuildText(const FText& InValue)
{
	if (false == BuildText.EqualTo(InValue))
	{
		BuildText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(BuildText);
	}
}

void UVX_VM_Result::SetRestartText(const FText& InValue)
{
	if (false == RestartText.EqualTo(InValue))
	{
		RestartText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RestartText);
	}
}

void UVX_VM_Result::SetQuitText(const FText& InValue)
{
	if (false == QuitText.EqualTo(InValue))
	{
		QuitText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(QuitText);
	}
}
