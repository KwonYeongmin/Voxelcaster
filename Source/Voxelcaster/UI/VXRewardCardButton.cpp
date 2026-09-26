// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXRewardCardButton.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/VXText.h"
#include "UI/VXUIBuilder.h"
#include "UI/ViewModel/VX_VM_RewardCard.h"

bool UVXRewardCardButton::Initialize()
{
	// CommonButtonBase가 루트를 버튼으로 감싸기 전에 트리가 있어야 한다. WBP면 이미 있다.
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
	if (nullptr == WidgetTree->RootWidget)
	{
		BuildDefaultTree();
		bBuiltInCode = true;
	}

	const bool bResult = Super::Initialize();

	SetIsFocusable(true);
	SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	return bResult;
}

void UVXRewardCardButton::BuildDefaultTree()
{
	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	Size->SetWidthOverride(280.f);
	Size->SetHeightOverride(340.f);
	WidgetTree->RootWidget = Size;

	CardBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	CardBorder->SetBrushColor(VXUI::Panel);
	CardBorder->SetPadding(FMargin(16.f));
	Size->AddChild(CardBorder);

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	CardBorder->AddChild(Box);

	const auto Add = [this, Box](int32 FontSize, const FLinearColor& Color, bool bBold, const FMargin& InPadding)
	{
		UTextBlock* Text = VXUI::MakeText(WidgetTree, FontSize, Color, bBold);
		Text->SetJustification(ETextJustify::Center);
		Text->SetAutoWrapText(true);
		UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Text);
		BoxSlot->SetPadding(InPadding);
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		return Text;
	};

	SkillText = Add(18, FLinearColor(0.7f, 0.8f, 1.f), false, FMargin(0, 8, 0, 4));
	ModifierText = Add(30, VXUI::Title, true, FMargin(0, 4, 0, 16));
	DescText = Add(16, VXUI::Body, false, FMargin(0, 8));
	TagText = Add(14, FLinearColor(0.4f, 1.f, 0.6f), true, FMargin(0, 16, 0, 0));
}

void UVXRewardCardButton::SetCard(const FVXUpgradeCard& InCard, int32 InIndex, UVX_VM_RewardCard* InViewModel)
{
	Card = InCard;
	CardIndex = InIndex;
	ViewModel = InViewModel;

	const FText Skill = FText::FromString(UVXModifierComponent::GetSkillDisplayName(Card.SkillTag));
	const FText Modifier = FText::FromString(UVXModifierComponent::GetModifierDisplayName(Card.Modifier));
	const FText Level = FText::FromString(VXText::Format(TEXT("UI.Level"), { Card.ResultStack }));
	const FText Desc = FText::FromString(Card.GetDescription());
	const FText Tag = FText::FromString(VXText::Get(Card.IsStackUpgrade() ? TEXT("UI.Upgrade") : TEXT("UI.New")));

	if (ViewModel)
	{
		ViewModel->SetSkillName(Skill);
		ViewModel->SetModifierName(Modifier);
		ViewModel->SetLevelText(Level);
		ViewModel->SetDescription(Desc);
		ViewModel->SetTagText(Tag);
		ViewModel->SetbUpgrade(Card.IsStackUpgrade());
		ViewModel->SetbHighlighted(false);
		VXUI::SetViewModel(this, ViewModel);
	}

	if (bBuiltInCode)
	{
		VXUI::SetText(SkillText, Skill);
		VXUI::SetText(ModifierText, FText::FromString(Modifier.ToString() + TEXT("  ") + Level.ToString()));
		VXUI::SetText(DescText, Desc);
		VXUI::SetText(TagText, Tag);
	}
}

void UVXRewardCardButton::SetHighlighted(bool bInHighlighted)
{
	if (ViewModel)
	{
		ViewModel->SetbHighlighted(bInHighlighted);
	}

	// 확대는 WBP·기본 트리 공통으로 코드에서 한다. 색은 기본 트리만 (WBP는 bHighlighted 바인딩)
	SetRenderScale(bInHighlighted ? FVector2D(1.1f, 1.1f) : FVector2D(1.f, 1.f));
	if (bBuiltInCode && CardBorder)
	{
		CardBorder->SetBrushColor(bInHighlighted ? VXUI::PanelHighlight : VXUI::Panel);
	}
}
