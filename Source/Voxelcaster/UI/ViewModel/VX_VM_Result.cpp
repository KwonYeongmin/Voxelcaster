// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Result.h"

void UVX_VM_Result::SetTitleText(const FText& InValue)
{
	VX_VM_SET_TEXT(TitleText, InValue);
}

void UVX_VM_Result::SetbVictory(bool InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(bVictory, InValue);
}

void UVX_VM_Result::SetStatsText(const FText& InValue)
{
	VX_VM_SET_TEXT(StatsText, InValue);
}

void UVX_VM_Result::SetBuildText(const FText& InValue)
{
	VX_VM_SET_TEXT(BuildText, InValue);
}

void UVX_VM_Result::SetRestartText(const FText& InValue)
{
	VX_VM_SET_TEXT(RestartText, InValue);
}

void UVX_VM_Result::SetQuitText(const FText& InValue)
{
	VX_VM_SET_TEXT(QuitText, InValue);
}
