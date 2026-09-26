// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXRewardSelectWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Input/UIActionBindingHandle.h"
#include "Modifier/VXModifierComponent.h"
#include "UI/VXRewardCardButton.h"
#include "UI/VXText.h"
#include "UI/VXUIBuilder.h"
#include "UI/ViewModel/VXRewardViewModel.h"

bool UVXRewardSelectWidget::Initialize()
{
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
	if (nullptr == WidgetTree->RootWidget)
	{
		BuildDefaultTree();
		bBuiltInCode = true;
	}

	// 뒤로가기 핸들러(bIsBackHandler)는 켜지 않는다. 켜면 CommonUI 뒤로가기 액션 데이터가 필요하다.
	// 이 화면은 레이어 스택에 올리지 않으므로 뒤로가기로 닫히지 않는다.
	return Super::Initialize();
}

void UVXRewardSelectWidget::BuildDefaultTree()
{
	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Dim->SetBrushColor(VXUI::Dim);
	Dim->SetHorizontalAlignment(HAlign_Center);
	Dim->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Dim;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Dim->AddChild(Box);

	const auto Add = [this, Box](int32 Size, const FLinearColor& Color, const FMargin& InPadding)
	{
		UTextBlock* Text = VXUI::MakeText(WidgetTree, Size, Color);
		Text->SetJustification(ETextJustify::Center);
		UVerticalBoxSlot* BoxSlot = Box->AddChildToVerticalBox(Text);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetPadding(InPadding);
		return Text;
	};

	TitleText = Add(32, VXUI::Title, FMargin(0, 0, 0, 32));

	CardRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Box->AddChildToVerticalBox(CardRow)->SetHorizontalAlignment(HAlign_Center);

	BuildText = Add(18, VXUI::Accent, FMargin(0, 40, 0, 8));
	HintText = Add(16, VXUI::Muted, FMargin(0, 8, 0, 0));
}

void UVXRewardSelectWidget::SetChoices(const TArray<FVXUpgradeCard>& InChoices, int32 WaveIndex)
{
	ViewModel = NewObject<UVXRewardViewModel>(this);
	ViewModel->SetTitleText(FText::FromString(VXText::Format(TEXT("UI.RewardTitle"), { WaveIndex })));
	ViewModel->SetHintText(FText::FromString(VXText::Get(TEXT("UI.RewardHint"))));
	ViewModel->SetCardCount(InChoices.Num());
	VXUI::SetViewModel(this, ViewModel);

	if (bBuiltInCode)
	{
		VXUI::SetText(TitleText, ViewModel->GetTitleText());
		VXUI::SetText(HintText, ViewModel->GetHintText());
	}

	// 카드 위젯 준비: WBP면 Card0~2, 기본 트리면 새로 만든다.
	Cards.Reset();
	if (bBuiltInCode)
	{
		CardRow->ClearChildren();
		for (int32 i = 0; i < InChoices.Num(); ++i)
		{
			UVXRewardCardButton* Card = CreateWidget<UVXRewardCardButton>(this, UVXRewardCardButton::StaticClass());
			UHorizontalBoxSlot* CardSlot = CardRow->AddChildToHorizontalBox(Card);
			CardSlot->SetPadding(FMargin(20.f, 0.f));
			CardSlot->SetVerticalAlignment(VAlign_Center);
			Cards.Add(Card);
		}
	}
	else
	{
		UVXRewardCardButton* Bound[] = { Card0, Card1, Card2 };
		for (int32 i = 0; i < UE_ARRAY_COUNT(Bound); ++i)
		{
			if (nullptr == Bound[i])
			{
				continue;
			}
			const bool bUsed = i < InChoices.Num();
			Bound[i]->SetVisibility(bUsed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (bUsed)
			{
				Cards.Add(Bound[i]);
			}
		}
	}

	UVXRewardCardViewModel* CardViewModels[] = { ViewModel->GetCard0(), ViewModel->GetCard1(), ViewModel->GetCard2() };
	for (int32 i = 0; i < Cards.Num() && i < InChoices.Num(); ++i)
	{
		UVXRewardCardButton* Card = Cards[i];
		Card->SetCard(InChoices[i], i, CardViewModels[i]);

		// 공개 네이티브 이벤트에 카드 자신을 인자로 넘겨 연결한다.
		Card->OnFocusReceived().AddUObject(this, &UVXRewardSelectWidget::HandleCardFocused, static_cast<UCommonButtonBase*>(Card));
		Card->OnHovered().AddUObject(this, &UVXRewardSelectWidget::HandleCardFocused, static_cast<UCommonButtonBase*>(Card));
		Card->OnClicked().AddUObject(this, &UVXRewardSelectWidget::HandleCardClicked, static_cast<UCommonButtonBase*>(Card));
	}

	if (Cards.Num() > 0)
	{
		HandleCardFocused(Cards[Cards.Num() / 2]);
	}
}

UWidget* UVXRewardSelectWidget::NativeGetDesiredFocusTarget() const
{
	// 가운데 카드 (DES-UI-REWARD-001 요구사항 1)
	return Cards.Num() > 0 ? Cards[Cards.Num() / 2].Get() : nullptr;
}

TOptional<FUIInputConfig> UVXRewardSelectWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UVXRewardSelectWidget::FocusDefaultCard()
{
	if (UWidget* Target = NativeGetDesiredFocusTarget())
	{
		Target->SetFocus();
	}
}

void UVXRewardSelectWidget::HandleCardFocused(UCommonButtonBase* Button)
{
	if (ConfirmTimer >= 0.f)
	{
		return;
	}

	for (UVXRewardCardButton* Card : Cards)
	{
		const bool bThis = Card == Button;
		Card->SetHighlighted(bThis);
		if (bThis)
		{
			UpdateBuildText(Card->GetCard());
		}
	}
}

void UVXRewardSelectWidget::UpdateBuildText(const FVXUpgradeCard& Card)
{
	FString Text = VXText::Format(TEXT("UI.BuildOf"), { UVXModifierComponent::GetSkillDisplayName(Card.SkillTag) });
	const APlayerController* PC = GetOwningPlayer();
	const UVXModifierComponent* Modifiers = (PC && PC->GetPawn()) ? PC->GetPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr;
	const FVXModifierSlots* Slots = nullptr != Modifiers ? Modifiers->FindSlots(Card.SkillTag) : nullptr;
	const int32 Used = nullptr != Slots ? Slots->Slots.Num() : 0;

	for (int32 i = 0; i < UVXModifierComponent::MaxSlots; ++i)
	{
		if (i < Used)
		{
			Text += FString::Printf(TEXT("  [%s]"), *UVXModifierComponent::GetModifierDisplayName(Slots->Slots[i]));
		}
		else if (i == Used)
		{
			// 이 카드를 고르면 들어갈 자리 (미리보기)
			Text += FString::Printf(TEXT("  [+%s]"), *UVXModifierComponent::GetModifierDisplayName(Card.Modifier));
		}
		else
		{
			Text += TEXT("  [   ]");
		}
	}

	if (ViewModel)
	{
		ViewModel->SetBuildText(FText::FromString(Text));
	}
	if (bBuiltInCode)
	{
		VXUI::SetText(BuildText, FText::FromString(Text));
	}
}

void UVXRewardSelectWidget::HandleCardClicked(UCommonButtonBase* Button)
{
	// 확인 연출 중 추가 입력은 무시한다. (중복 선택 방지)
	if (ConfirmTimer >= 0.f)
	{
		return;
	}

	const UVXRewardCardButton* Card = Cast<UVXRewardCardButton>(Button);
	if (nullptr == Card)
	{
		return;
	}

	ChosenIndex = Card->GetCardIndex();
	ConfirmTimer = ConfirmDelay;

	// 확정 연출: 고른 카드만 남기고 나머지를 흐리게
	for (UVXRewardCardButton* Other : Cards)
	{
		Other->SetRenderOpacity(Other == Card ? 1.f : 0.25f);
	}
}

void UVXRewardSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 위젯 틱은 게임이 일시정지되어도 돌기 때문에 연출 타이머를 여기서 센다.
	if (ConfirmTimer >= 0.f)
	{
		ConfirmTimer -= InDeltaTime;
		if (ConfirmTimer < 0.f)
		{
			FinishSelection();
		}
	}
}

void UVXRewardSelectWidget::FinishSelection()
{
	const int32 Index = ChosenIndex;
	UWorld* World = GetWorld();

	DeactivateWidget();
	RemoveFromParent();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(false);
		PC->SetInputMode(InputMode);
	}

	// 게임을 재개한 뒤 선택을 적용한다. (다음 웨이브가 이어서 시작된다)
	if (UVXUpgradeSubsystem* Upgrades = nullptr != World ? World->GetSubsystem<UVXUpgradeSubsystem>() : nullptr)
	{
		Upgrades->ApplyChoice(Index);
	}
}
