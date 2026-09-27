// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXUIBuilder.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
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
		// CommonUI 규칙: 글자는 CommonTextBlock (TextBlock을 상속하므로 반환 타입은 그대로)
		UTextBlock* Text = Tree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass());
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

	bool HasDesignerTree(const UUserWidget* Widget)
	{
		const UWidgetBlueprintGeneratedClass* Class = nullptr != Widget ? Cast<UWidgetBlueprintGeneratedClass>(Widget->GetClass()) : nullptr;
		const UWidgetTree* Archetype = nullptr != Class ? Class->GetWidgetTreeArchetype() : nullptr;
		return nullptr != Archetype && nullptr != Archetype->RootWidget;
	}

	void SetText(UTextBlock* Text, const FText& Value)
	{
		if (Text)
		{
			Text->SetText(Value);
		}
	}
}
