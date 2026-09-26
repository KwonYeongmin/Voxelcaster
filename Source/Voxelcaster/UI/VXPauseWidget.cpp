// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXPauseWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Input/UIActionBindingHandle.h"
#include "Player/VXPlayerController.h"
#include "UI/VXMenuButton.h"
#include "UI/VXText.h"
#include "UI/VXUIBuilder.h"
#include "UI/ViewModel/VX_VM_Pause.h"

bool UVXPauseWidget::Initialize()
{
	if (nullptr == WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}
	if (nullptr == WidgetTree->RootWidget)
	{
		BuildDefaultTree();
	}
	return Super::Initialize();
}

void UVXPauseWidget::BuildDefaultTree()
{
	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Dim->SetBrushColor(VXUI::Dim);
	Dim->SetHorizontalAlignment(HAlign_Center);
	Dim->SetVerticalAlignment(VAlign_Center);
	WidgetTree->RootWidget = Dim;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Dim->AddChild(Box);

	UTextBlock* Title = VXUI::MakeText(WidgetTree, 40, VXUI::Title);
	Title->SetText(FText::FromString(VXText::Get(TEXT("UI.Paused"))));
	UVerticalBoxSlot* TitleSlot = Box->AddChildToVerticalBox(Title);
	TitleSlot->SetHorizontalAlignment(HAlign_Center);
	TitleSlot->SetPadding(FMargin(0, 0, 0, 32));

	const auto AddButton = [this, Box](const TCHAR* Key)
	{
		UVXMenuButton* Button = CreateWidget<UVXMenuButton>(this, UVXMenuButton::StaticClass());
		Button->SetLabel(VXText::Get(Key));
		UVerticalBoxSlot* ButtonSlot = Box->AddChildToVerticalBox(Button);
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonSlot->SetPadding(FMargin(0, 8));
		return Button;
	};

	ResumeButton = AddButton(TEXT("UI.Resume"));
	RestartButton = AddButton(TEXT("UI.Restart"));
	QuitButton = AddButton(TEXT("UI.Quit"));
}

void UVXPauseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 글자는 뷰모델로 WBP에 전달한다.
	ViewModel = NewObject<UVX_VM_Pause>(this);
	ViewModel->SetTitleText(FText::FromString(VXText::Get(TEXT("UI.Paused"))));
	ViewModel->SetResumeText(FText::FromString(VXText::Get(TEXT("UI.Resume"))));
	ViewModel->SetRestartText(FText::FromString(VXText::Get(TEXT("UI.Restart"))));
	ViewModel->SetQuitText(FText::FromString(VXText::Get(TEXT("UI.Quit"))));
	VXUI::SetViewModel(this, ViewModel);

	// 메뉴 버튼 글자 (버튼은 별도 WBP라 부모 뷰모델에 바인딩할 수 없어서 코드로 넣는다)
	if (UVXMenuButton* Button = Cast<UVXMenuButton>(ResumeButton))
	{
		Button->SetLabel(ViewModel->GetResumeText().ToString());
	}
	if (UVXMenuButton* Button = Cast<UVXMenuButton>(RestartButton))
	{
		Button->SetLabel(ViewModel->GetRestartText().ToString());
	}
	if (UVXMenuButton* Button = Cast<UVXMenuButton>(QuitButton))
	{
		Button->SetLabel(ViewModel->GetQuitText().ToString());
	}

	if (ResumeButton)
	{
		ResumeButton->OnClicked().AddUObject(this, &UVXPauseWidget::HandleResume);
	}
	if (RestartButton)
	{
		RestartButton->OnClicked().AddUObject(this, &UVXPauseWidget::HandleRestart);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UVXPauseWidget::HandleQuit);
	}
}

UWidget* UVXPauseWidget::NativeGetDesiredFocusTarget() const
{
	return ResumeButton;
}

TOptional<FUIInputConfig> UVXPauseWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

FReply UVXPauseWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 버튼이 처리하지 않은 키가 여기로 올라온다. Esc · Menu · B 로 닫는다.
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Gamepad_FaceButton_Right)
	{
		HandleResume();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UVXPauseWidget::HandleResume()
{
	if (AVXPlayerController* PC = Cast<AVXPlayerController>(GetOwningPlayer()))
	{
		PC->CloseMenu();
	}
}

void UVXPauseWidget::HandleRestart()
{
	if (AVXPlayerController* PC = Cast<AVXPlayerController>(GetOwningPlayer()))
	{
		PC->RestartGame();
	}
}

void UVXPauseWidget::HandleQuit()
{
	if (AVXPlayerController* PC = Cast<AVXPlayerController>(GetOwningPlayer()))
	{
		PC->QuitGame();
	}
}
