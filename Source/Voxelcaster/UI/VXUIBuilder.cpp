// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXUIBuilder.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "MVVMSubsystem.h"
#include "MVVMViewModelBase.h"
#include "Styling/CoreStyle.h"
#include "View/MVVMView.h"

namespace VXUI
{
	const FLinearColor Title(1.f, 0.85f, 0.4f);
	const FLinearColor Body(1.f, 1.f, 1.f);
	const FLinearColor Muted(0.7f, 0.7f, 0.75f);
	const FLinearColor Accent(0.55f, 0.85f, 1.f);
	const FLinearColor Panel(0.06f, 0.05f, 0.09f, 0.9f);
	const FLinearColor PanelHighlight(0.28f, 0.18f, 0.45f, 1.f);
	const FLinearColor Dim(0.f, 0.f, 0.f, 0.7f);

	UTextBlock* MakeText(UWidgetTree* Tree, int32 Size, const FLinearColor& Color, bool bBold)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
		Text->SetColorAndOpacity(FSlateColor(Color));
		return Text;
	}

	void SetViewModel(UUserWidget* Widget, UMVVMViewModelBase* ViewModel)
	{
		if (nullptr == Widget || nullptr == ViewModel)
		{
			return;
		}

		if (UMVVMView* View = UMVVMSubsystem::GetViewFromUserWidget(Widget))
		{
			View->SetViewModelByClass(TScriptInterface<INotifyFieldValueChanged>(ViewModel));
		}
	}

	void SetText(UTextBlock* Text, const FText& Value)
	{
		if (Text)
		{
			Text->SetText(Value);
		}
	}
}
