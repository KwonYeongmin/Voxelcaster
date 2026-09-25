// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXRewardCardButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

namespace
{
	UTextBlock* MakeText(UWidgetTree* Tree, UVerticalBox* Parent, int32 Size, const FLinearColor& Color, bool bBold, const FMargin& Padding)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetJustification(ETextJustify::Center);
		Text->SetAutoWrapText(true);
		UVerticalBoxSlot* BoxSlot = Parent->AddChildToVerticalBox(Text);
		BoxSlot->SetPadding(Padding);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		return Text;
	}

	const FLinearColor CardColor(0.06f, 0.05f, 0.09f, 0.95f);
	const FLinearColor CardHighlightColor(0.28f, 0.18f, 0.45f, 1.f);
}

bool UVXRewardCardButton::Initialize()
{
	// 디자이너 에셋이 없으므로 CommonButtonBase가 루트를 버튼으로 감싸기 전에 위젯 트리를 직접 만든다.
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (nullptr == WidgetTree->RootWidget)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(280.f);
		Size->SetHeightOverride(340.f);
		WidgetTree->RootWidget = Size;

		CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		CardBorder->SetBrushColor(CardColor);
		CardBorder->SetPadding(FMargin(16.f));
		Size->AddChild(CardBorder);

		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		CardBorder->AddChild(Box);

		SkillText = MakeText(WidgetTree, Box, 18, FLinearColor(0.7f, 0.8f, 1.f), false, FMargin(0, 8, 0, 4));
		ModifierText = MakeText(WidgetTree, Box, 30, FLinearColor(1.f, 0.85f, 0.4f), true, FMargin(0, 4, 0, 16));
		DescText = MakeText(WidgetTree, Box, 16, FLinearColor::White, false, FMargin(0, 8));
		TagText = MakeText(WidgetTree, Box, 14, FLinearColor(0.4f, 1.f, 0.6f), true, FMargin(0, 16, 0, 0));
	}

	const bool bResult = Super::Initialize();

	SetIsFocusable(true);
	SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	return bResult;
}

void UVXRewardCardButton::SetCard(const FVXUpgradeCard& InCard, int32 InIndex)
{
	Card = InCard;
	CardIndex = InIndex;

	if (SkillText)
	{
		SkillText->SetText(FText::FromString(UVXModifierComponent::GetSkillName(Card.SkillTag)));
	}
	if (ModifierText)
	{
		ModifierText->SetText(FText::FromString(FString::Printf(TEXT("%s  Lv.%d"),
			*UVXModifierComponent::GetModifierName(Card.Modifier), Card.ResultStack)));
	}
	if (DescText)
	{
		DescText->SetText(FText::FromString(Card.GetDescription()));
	}
	if (TagText)
	{
		TagText->SetText(Card.IsStackUpgrade() ? FText::FromString(TEXT("UPGRADE")) : FText::FromString(TEXT("NEW")));
	}
}

void UVXRewardCardButton::SetHighlighted(bool bInHighlighted)
{
	SetRenderScale(bInHighlighted ? FVector2D(1.1f, 1.1f) : FVector2D(1.f, 1.f));
	if (CardBorder)
	{
		CardBorder->SetBrushColor(bInHighlighted ? CardHighlightColor : CardColor);
	}
}
