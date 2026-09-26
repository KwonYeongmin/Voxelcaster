// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Pause.h"

void UVX_VM_Pause::SetTitleText(const FText& InValue)
{
	VX_VM_SET_TEXT(TitleText, InValue);
}

void UVX_VM_Pause::SetResumeText(const FText& InValue)
{
	VX_VM_SET_TEXT(ResumeText, InValue);
}

void UVX_VM_Pause::SetRestartText(const FText& InValue)
{
	VX_VM_SET_TEXT(RestartText, InValue);
}

void UVX_VM_Pause::SetQuitText(const FText& InValue)
{
	VX_VM_SET_TEXT(QuitText, InValue);
}
