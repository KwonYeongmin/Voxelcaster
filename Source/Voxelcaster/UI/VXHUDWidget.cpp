// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXHUDWidget.h"
#include "CommonBorder.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "UI/VXUIBuilder.h"
#include "UI/ViewModel/VX_VM_Hud.h"

namespace
{
	UTextBlock* AddToVBox(UWidgetTree* Tree, UVerticalBox* Box, int32 Size, const FLinearColor& Color, bool bBold = true)
	{
		UTextBlock* Text = VXUI::MakeText(Tree, Size, Color, bBold);
		Text->SetJustification(ETextJustify::Center);
		UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Text);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		return Text;
	}
}

bool UVXHUDWidget::Initialize()
{
	if (false == VXUI::HasDesignerTree(this) && nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	// WBP_VX_HUD면 디자이너가 만든 트리가 이미 있다. 없을 때만 기본 트리를 만든다.
	if (false == VXUI::HasDesignerTree(this) && nullptr == WidgetTree->RootWidget)
	{
		BuildDefaultTree();
		bBuiltInCode = true;
	}

	return Super::Initialize();
}

void UVXHUDWidget::SetViewModel(UVX_VM_Hud* InViewModel)
{
	ViewModel = InViewModel;
	VXUI::SetViewModel(this, InViewModel);
}

void UVXHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// WBP는 View Bindings가 갱신한다. C++ 기본 트리만 뷰모델을 직접 읽는다.
	if (bBuiltInCode)
	{
		RefreshDefaultTree();
	}
}

void UVXHUDWidget::RefreshDefaultTree()
{
	if (nullptr == ViewModel)
	{
		return;
	}

	HealthBar->SetPercent(ViewModel->GetHealthPercent());
	HealthBar->SetFillColorAndOpacity(ViewModel->GetbLowHealth() ? FLinearColor(0.95f, 0.2f, 0.15f) : FLinearColor(0.2f, 0.85f, 0.35f));
	HealthText->SetText(ViewModel->GetHealthText());
	WaveText->SetText(ViewModel->GetWaveText());
	EnemyText->SetText(ViewModel->GetEnemiesText());

	const float BannerOpacity = ViewModel->GetBannerOpacity();
	BannerBox->SetVisibility(BannerOpacity > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	BannerBox->SetRenderOpacity(BannerOpacity);
	BannerText->SetText(ViewModel->GetBannerText());
	BannerSubText->SetText(ViewModel->GetBannerSubText());

	const UVX_VM_SkillSlot* Slots[] = { ViewModel->GetSkillSlot0(), ViewModel->GetSkillSlot1(), ViewModel->GetSkillSlot2(), ViewModel->GetSkillSlot3() };
	for (int32 i = 0; i < SlotWidgets.Num() && i < UE_ARRAY_COUNT(Slots); ++i)
	{
		RefreshSlot(SlotWidgets[i], Slots[i]);
	}
}

void UVXHUDWidget::RefreshSlot(FVXHUDSkillSlotWidgets& Widgets, const UVX_VM_SkillSlot* SlotViewModel) const
{
	if (nullptr == SlotViewModel)
	{
		return;
	}

	Widgets.KeyText->SetText(SlotViewModel->GetKeyText());
	Widgets.NameText->SetText(SlotViewModel->GetSkillName());
	Widgets.NameText->SetColorAndOpacity(FSlateColor(SlotViewModel->GetbReady() ? VXUI::Body : VXUI::Muted));
	Widgets.CooldownBar->SetPercent(SlotViewModel->GetCooldownPercent());
	Widgets.CooldownText->SetText(SlotViewModel->GetCooldownText());
	Widgets.ModifierText->SetText(SlotViewModel->GetModifiersText());
}

void UVXHUDWidget::BuildDefaultTree()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	WidgetTree->RootWidget = Root;

	// ---- 상단 중앙: 웨이브
	{
		UVerticalBox* Top = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UOverlaySlot* TopSlot = Root->AddChildToOverlay(Top);
		TopSlot->SetHorizontalAlignment(HAlign_Center);
		TopSlot->SetVerticalAlignment(VAlign_Top);
		TopSlot->SetPadding(FMargin(0, 24, 0, 0));

		WaveText = AddToVBox(WidgetTree, Top, 30, VXUI::Title);
		EnemyText = AddToVBox(WidgetTree, Top, 18, VXUI::Body, false);
	}

	// ---- 화면 위쪽 가운데: 웨이브 시작 배너
	{
		UVerticalBox* Banner = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UOverlaySlot* BannerSlot = Root->AddChildToOverlay(Banner);
		BannerSlot->SetHorizontalAlignment(HAlign_Center);
		BannerSlot->SetVerticalAlignment(VAlign_Center);
		BannerSlot->SetPadding(FMargin(0, 0, 0, 280));
		Banner->SetVisibility(ESlateVisibility::Collapsed);

		BannerText = AddToVBox(WidgetTree, Banner, 64, VXUI::Title);
		BannerText->SetShadowOffset(FVector2D(3.f, 3.f));
		BannerText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
		BannerSubText = AddToVBox(WidgetTree, Banner, 24, VXUI::Accent);
		BannerSubText->SetShadowOffset(FVector2D(2.f, 2.f));
		BannerSubText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.8f));
		BannerBox = Banner;
	}

	// ---- 좌측 하단: 체력
	{
		UVerticalBox* HealthBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UOverlaySlot* HealthSlot = Root->AddChildToOverlay(HealthBox);
		HealthSlot->SetHorizontalAlignment(HAlign_Left);
		HealthSlot->SetVerticalAlignment(VAlign_Bottom);
		HealthSlot->SetPadding(FMargin(40, 0, 0, 40));

		HealthText = VXUI::MakeText(WidgetTree, 22, VXUI::Body);
		HealthBox->AddChildToVerticalBox(HealthText)->SetPadding(FMargin(0, 0, 0, 6));

		USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		BarSize->SetWidthOverride(320.f);
		BarSize->SetHeightOverride(18.f);
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		BarSize->AddChild(HealthBar);
		HealthBox->AddChildToVerticalBox(BarSize);
	}

	// ---- 하단 중앙: 스킬 슬롯 4개 (스킬 3 + 대시)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UOverlaySlot* RowSlot = Root->AddChildToOverlay(Row);
		RowSlot->SetHorizontalAlignment(HAlign_Center);
		RowSlot->SetVerticalAlignment(VAlign_Bottom);
		RowSlot->SetPadding(FMargin(0, 0, 0, 32));

		for (int32 i = 0; i < 4; ++i)
		{
			FVXHUDSkillSlotWidgets Widgets;

			USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			Size->SetWidthOverride(170.f);
			UHorizontalBoxSlot* SizeSlot = Row->AddChildToHorizontalBox(Size);
			SizeSlot->SetPadding(FMargin(8, 0));
			SizeSlot->SetVerticalAlignment(VAlign_Bottom);

			UBorder* Panel = WidgetTree->ConstructWidget<UCommonBorder>(UCommonBorder::StaticClass());
			Panel->SetBrushColor(VXUI::Panel);
			Panel->SetPadding(FMargin(10));
			Size->AddChild(Panel);

			UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
			Panel->AddChild(Box);

			Widgets.KeyText = AddToVBox(WidgetTree, Box, 14, VXUI::Muted);
			Widgets.NameText = AddToVBox(WidgetTree, Box, 18, VXUI::Body);

			USizeBox* BarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			BarSize->SetHeightOverride(6.f);
			Widgets.CooldownBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
			Widgets.CooldownBar->SetFillColorAndOpacity(VXUI::Accent);
			BarSize->AddChild(Widgets.CooldownBar);
			Box->AddChildToVerticalBox(BarSize)->SetPadding(FMargin(0, 6, 0, 2));

			Widgets.CooldownText = AddToVBox(WidgetTree, Box, 14, VXUI::Accent);
			Widgets.ModifierText = AddToVBox(WidgetTree, Box, 13, VXUI::Title, false);
			Widgets.ModifierText->SetAutoWrapText(true);

			SlotWidgets.Add(Widgets);
		}
	}
}
