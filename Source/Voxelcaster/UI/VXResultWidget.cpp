// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXResultWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GAS/VXGameplayTags.h"
#include "Input/UIActionBindingHandle.h"
#include "Modifier/VXModifierComponent.h"
#include "Player/VXPlayerController.h"
#include "UI/VXMenuButton.h"
#include "UI/VXText.h"
#include "UI/VXUIBuilder.h"
#include "UI/ViewModel/VX_VM_Result.h"

bool UVXResultWidget::Initialize()
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
	return Super::Initialize();
}

void UVXResultWidget::BuildDefaultTree()
{
	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Dim->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.8f));
	Dim->SetHorizontalAlignment(HAlign_Center);
	Dim->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Dim;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Dim->AddChild(Box);

	const auto AddText = [this, Box](int32 Size, const FLinearColor& Color, const FMargin& InPadding)
	{
		UTextBlock* Text = VXUI::MakeText(WidgetTree, Size, Color);
		Text->SetJustification(ETextJustify::Center);
		UVerticalBoxSlot* TextSlot = Box->AddChildToVerticalBox(Text);
		TextSlot->SetHorizontalAlignment(HAlign_Center);
		TextSlot->SetPadding(InPadding);
		return Text;
	};

	TitleText = AddText(56, VXUI::Title, FMargin(0, 0, 0, 24));
	StatsText = AddText(22, VXUI::Body, FMargin(0, 0, 0, 24));
	BuildText = AddText(20, VXUI::Accent, FMargin(0, 0, 0, 32));

	const auto AddButton = [this, Box](const TCHAR* Key)
	{
		UVXMenuButton* Button = CreateWidget<UVXMenuButton>(this, UVXMenuButton::StaticClass());
		Button->SetLabel(VXText::Get(Key));
		UVerticalBoxSlot* ButtonSlot = Box->AddChildToVerticalBox(Button);
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonSlot->SetPadding(FMargin(0, 8));
		return Button;
	};

	RestartButton = AddButton(TEXT("UI.Restart"));
	QuitButton = AddButton(TEXT("UI.Quit"));
}

void UVXResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ViewModel = NewObject<UVX_VM_Result>(this);
	ViewModel->SetRestartText(FText::FromString(VXText::Get(TEXT("UI.Restart"))));
	ViewModel->SetQuitText(FText::FromString(VXText::Get(TEXT("UI.Quit"))));
	VXUI::SetViewModel(this, ViewModel);

	// 메뉴 버튼 글자 (버튼은 별도 WBP라 부모 뷰모델에 바인딩할 수 없어서 코드로 넣는다)
	if (UVXMenuButton* Button = Cast<UVXMenuButton>(RestartButton))
	{
		Button->SetLabel(ViewModel->GetRestartText().ToString());
	}
	if (UVXMenuButton* Button = Cast<UVXMenuButton>(QuitButton))
	{
		Button->SetLabel(ViewModel->GetQuitText().ToString());
	}

	if (RestartButton)
	{
		RestartButton->OnClicked().AddUObject(this, &UVXResultWidget::HandleRestart);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UVXResultWidget::HandleQuit);
	}
}

void UVXResultWidget::SetResult(const FVXRunResult& Result, const UVXModifierComponent* Modifiers)
{
	const int32 Seconds = FMath::FloorToInt(Result.PlayTime);
	const FString Time = FString::Printf(TEXT("%d:%02d"), Seconds / 60, Seconds % 60);
	const FString Stats =
		VXText::Format(TEXT("UI.ReachedWave"), { Result.ReachedWave, Result.TotalWaves }) + TEXT("\n") +
		VXText::Format(TEXT("UI.Kills"), { Result.Kills }) + TEXT("\n") +
		VXText::Format(TEXT("UI.PlayTime"), { Time });

	// 최종 빌드: 스킬 3개와 장착 모디파이어 (영상에서 한눈에 읽혀야 한다)
	FString Build = VXText::Get(TEXT("UI.FinalBuild"));
	const FGameplayTag Skills[] = { VXTags::Cooldown_MagicBolt, VXTags::Cooldown_Nova, VXTags::Cooldown_BladeSweep };
	for (const FGameplayTag& Skill : Skills)
	{
		Build += FString::Printf(TEXT("\n%s :"), *UVXModifierComponent::GetSkillDisplayName(Skill));
		const FVXModifierSlots* Slots = nullptr != Modifiers ? Modifiers->FindSlots(Skill) : nullptr;
		if (nullptr == Slots || Slots->Slots.IsEmpty())
		{
			Build += TEXT("  -");
			continue;
		}
		for (const EVXModifierType Type : Slots->Slots)
		{
			Build += FString::Printf(TEXT("  [%s]"), *UVXModifierComponent::GetModifierDisplayName(Type));
		}
	}

	const FText Title = FText::FromString(VXText::Get(Result.bVictory ? TEXT("UI.Victory") : TEXT("UI.Defeat")));
	ViewModel->SetTitleText(Title);
	ViewModel->SetbVictory(Result.bVictory);
	ViewModel->SetStatsText(FText::FromString(Stats));
	ViewModel->SetBuildText(FText::FromString(Build));

	if (bBuiltInCode)
	{
		VXUI::SetText(TitleText, Title);
		TitleText->SetColorAndOpacity(FSlateColor(Result.bVictory ? VXUI::Title : FLinearColor(1.f, 0.35f, 0.3f)));
		VXUI::SetText(StatsText, FText::FromString(Stats));
		VXUI::SetText(BuildText, FText::FromString(Build));
	}
}

UWidget* UVXResultWidget::NativeGetDesiredFocusTarget() const
{
	return RestartButton;
}

TOptional<FUIInputConfig> UVXResultWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

void UVXResultWidget::HandleRestart()
{
	if (AVXPlayerController* PC = Cast<AVXPlayerController>(GetOwningPlayer()))
	{
		PC->RestartGame();
	}
}

void UVXResultWidget::HandleQuit()
{
	if (AVXPlayerController* PC = Cast<AVXPlayerController>(GetOwningPlayer()))
	{
		PC->QuitGame();
	}
}
