// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXMenuButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "UI/VXUIBuilder.h"

bool UVXMenuButton::Initialize()
{
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (nullptr == WidgetTree->RootWidget)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(320.f);
		Size->SetHeightOverride(56.f);
		WidgetTree->RootWidget = Size;

		ButtonBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		ButtonBorder->SetBrushColor(VXUI::Panel);
		ButtonBorder->SetHorizontalAlignment(HAlign_Center);
		ButtonBorder->SetVerticalAlignment(VAlign_Center);
		Size->AddChild(ButtonBorder);

		LabelText = VXUI::MakeText(WidgetTree, 22, VXUI::Body);
		ButtonBorder->AddChild(LabelText);
	}

	const bool bResult = Super::Initialize();

	SetIsFocusable(true);
	OnFocusReceived().AddUObject(this, &UVXMenuButton::SetHighlighted, true);
	OnFocusLost().AddUObject(this, &UVXMenuButton::SetHighlighted, false);
	OnHovered().AddUObject(this, &UVXMenuButton::SetHighlighted, true);
	OnUnhovered().AddUObject(this, &UVXMenuButton::SetHighlighted, false);
	return bResult;
}

void UVXMenuButton::SetLabel(const FString& Label)
{
	if (LabelText)
	{
		LabelText->SetText(FText::FromString(Label));
	}
}

void UVXMenuButton::SetHighlighted(bool bInHighlighted)
{
	if (ButtonBorder)
	{
		ButtonBorder->SetBrushColor(bInHighlighted ? VXUI::PanelHighlight : VXUI::Panel);
	}
	if (LabelText)
	{
		LabelText->SetColorAndOpacity(FSlateColor(bInHighlighted ? VXUI::Title : VXUI::Body));
	}
}
