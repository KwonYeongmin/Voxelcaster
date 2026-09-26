// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Pause.h"

void UVX_VM_Pause::SetTitleText(const FText& InValue)
{
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVX_VM_Pause::SetResumeText(const FText& InValue)
{
	if (false == ResumeText.EqualTo(InValue))
	{
		ResumeText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(ResumeText);
	}
}

void UVX_VM_Pause::SetRestartText(const FText& InValue)
{
	if (false == RestartText.EqualTo(InValue))
	{
		RestartText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(RestartText);
	}
}

void UVX_VM_Pause::SetQuitText(const FText& InValue)
{
	if (false == QuitText.EqualTo(InValue))
	{
		QuitText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(QuitText);
	}
}
