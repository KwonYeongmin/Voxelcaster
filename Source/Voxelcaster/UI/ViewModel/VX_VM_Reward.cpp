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
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVX_VM_Reward::SetHintText(const FText& InValue)
{
	if (false == HintText.EqualTo(InValue))
	{
		HintText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HintText);
	}
}

void UVX_VM_Reward::SetBuildText(const FText& InValue)
{
	if (false == BuildText.EqualTo(InValue))
	{
		BuildText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(BuildText);
	}
}

void UVX_VM_Reward::SetCardCount(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CardCount, InValue);
}
