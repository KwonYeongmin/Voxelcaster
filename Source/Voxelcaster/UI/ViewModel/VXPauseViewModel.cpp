// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXPauseViewModel.h"

void UVXPauseViewModel::SetTitleText(const FText& InValue)
{
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVXPauseViewModel::SetResumeText(const FText& InValue)
{
	if (false == ResumeText.EqualTo(InValue))
	{
		ResumeText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ResumeText);
	}
}

void UVXPauseViewModel::SetRestartText(const FText& InValue)
{
	if (false == RestartText.EqualTo(InValue))
	{
		RestartText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RestartText);
	}
}

void UVXPauseViewModel::SetQuitText(const FText& InValue)
{
	if (false == QuitText.EqualTo(InValue))
	{
		QuitText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(QuitText);
	}
}
