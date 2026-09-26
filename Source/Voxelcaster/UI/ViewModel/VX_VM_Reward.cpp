// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Reward.h"

UVX_VM_Reward::UVX_VM_Reward()
{
	Card0 = CreateDefaultSubobject<UVX_VM_RewardCard>(TEXT("Card0"));
	Card1 = CreateDefaultSubobject<UVX_VM_RewardCard>(TEXT("Card1"));
	Card2 = CreateDefaultSubobject<UVX_VM_RewardCard>(TEXT("Card2"));
}

void UVX_VM_Reward::SetTitleText(const FText& InValue)
{
	VX_VM_SET_TEXT(TitleText, InValue);
}

void UVX_VM_Reward::SetHintText(const FText& InValue)
{
	VX_VM_SET_TEXT(HintText, InValue);
}

void UVX_VM_Reward::SetBuildText(const FText& InValue)
{
	VX_VM_SET_TEXT(BuildText, InValue);
}

void UVX_VM_Reward::SetCardCount(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CardCount, InValue);
}
