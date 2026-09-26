// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VXRewardViewModel.h"

UVXRewardViewModel::UVXRewardViewModel()
{
	Card0 = CreateDefaultSubobject<UVXRewardCardViewModel>(TEXT("Card0"));
	Card1 = CreateDefaultSubobject<UVXRewardCardViewModel>(TEXT("Card1"));
	Card2 = CreateDefaultSubobject<UVXRewardCardViewModel>(TEXT("Card2"));
}

void UVXRewardViewModel::SetTitleText(const FText& InValue)
{
	if (false == TitleText.EqualTo(InValue))
	{
		TitleText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TitleText);
	}
}

void UVXRewardViewModel::SetHintText(const FText& InValue)
{
	if (false == HintText.EqualTo(InValue))
	{
		HintText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(HintText);
	}
}

void UVXRewardViewModel::SetBuildText(const FText& InValue)
{
	if (false == BuildText.EqualTo(InValue))
	{
		BuildText = InValue;
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(BuildText);
	}
}

void UVXRewardViewModel::SetCardCount(int32 InValue)
{
	UE_MVVM_SET_PROPERTY_VALUE(CardCount, InValue);
}
