// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXHealthBarWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

bool UVXHealthBarWidget::Initialize()
{
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (nullptr == WidgetTree->RootWidget)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(70.f);
		Size->SetHeightOverride(7.f);
		WidgetTree->RootWidget = Size;

		Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		Bar->SetPercent(1.f);
		Bar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.15f, 0.1f));
		Size->AddChild(Bar);
	}

	return Super::Initialize();
}

void UVXHealthBarWidget::SetHealth(float Current, float Max)
{
	if (Bar)
	{
		Bar->SetPercent(Max > 0.f ? FMath::Clamp(Current / Max, 0.f, 1.f) : 0.f);
	}
}
