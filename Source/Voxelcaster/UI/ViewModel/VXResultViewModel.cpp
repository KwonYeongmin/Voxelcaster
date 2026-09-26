// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXResultViewModel.h"

void UVXResultViewModel::SetTitleText(const FText& InValue)
{
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVXResultViewModel::SetbVictory(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bVictory, InValue);
}

void UVXResultViewModel::SetStatsText(const FText& InValue)
{
	if (false == StatsText.EqualTo(InValue))
	{
		StatsText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(StatsText);
	}
}

void UVXResultViewModel::SetBuildText(const FText& InValue)
{
	if (false == BuildText.EqualTo(InValue))
	{
		BuildText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(BuildText);
	}
}

void UVXResultViewModel::SetRestartText(const FText& InValue)
{
	if (false == RestartText.EqualTo(InValue))
	{
		RestartText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RestartText);
	}
}

void UVXResultViewModel::SetQuitText(const FText& InValue)
{
	if (false == QuitText.EqualTo(InValue))
	{
		QuitText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(QuitText);
	}
}
