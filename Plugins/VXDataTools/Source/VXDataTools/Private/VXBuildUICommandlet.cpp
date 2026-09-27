// Copyright Epic Games, Inc. All Rights Reserved.

#include "VXBuildUICommandlet.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "MVVMBlueprintView.h"
#include "MVVMBlueprintViewModelContext.h"
#include "MVVMEditorSubsystem.h"
#include "MVVMPropertyPath.h"
#include "Types/MVVMFieldVariant.h"
#include "VXDataReimporter.h"
#include "WidgetBlueprint.h"

namespace
{
	const FLinearColor AccentColor(0.55f, 0.85f, 1.f);

	UClass* FindGameClass(const TCHAR* Name)
	{
		return LoadClass<UObject>(nullptr, *FString::Printf(TEXT("/Script/Voxelcaster.%s"), Name));
	}

	UWidgetBlueprint* LoadWBP(const TCHAR* Name)
	{
		return LoadObject<UWidgetBlueprint>(nullptr, *FString::Printf(TEXT("/Game/Voxelcaster/UI/%s.%s"), Name, Name));
	}

	/**
	 * 위젯 이름: 코드가 이름으로 찾는 위젯(BindWidgetOptional)은 그대로, 나머지는 W_ 접두어.
	 * C++ 부모 클래스에 같은 이름의 변수(기본 화면용)가 있으면 WBP 컴파일이 실패하기 때문이다.
	 */
	FName ResolveName(FName Name)
	{
		static const TSet<FName> Required = { TEXT("Card0"), TEXT("Card1"), TEXT("Card2"), TEXT("ResumeButton"),
			TEXT("RestartButton"), TEXT("QuitButton"), TEXT("LabelText"), TEXT("ButtonBorder") };
		return Required.Contains(Name) ? Name : FName(*(TEXT("W_") + Name.ToString()));
	}

	/** WBP 하나의 위젯 트리와 바인딩을 만드는 도우미 */
	struct FUIBuilder
	{
		UWidgetBlueprint* BP = nullptr;
		UMVVMEditorSubsystem* MVVM = nullptr;
		FGuid ViewModelId;
		UClass* ViewModelClass = nullptr;
		int32 Errors = 0;

		UWidgetTree* Tree() const { return BP->WidgetTree; }

		/** 기존 위젯·바인딩을 지운다 */
		void Reset()
		{
			if (UMVVMBlueprintView* View = MVVM->RequestView(BP))
			{
				View->Modify();
				for (int32 Index = View->GetBindings().Num() - 1; Index >= 0; --Index)
				{
					View->RemoveBindingAt(Index);
				}
			}

			TArray<UWidget*> Old;
			Tree()->GetAllWidgets(Old);
			for (UWidget* Widget : Old)
			{
				BP->OnVariableRemoved(Widget->GetFName());
				Widget->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors | REN_NonTransactional);
			}
			Tree()->RootWidget = nullptr;
		}

		/** WBP에 추가된 뷰모델을 찾는다 (없으면 Manual로 추가) */
		bool UseViewModel(const TCHAR* ClassName)
		{
			ViewModelClass = FindGameClass(ClassName);
			if (nullptr == ViewModelClass)
			{
				UE_LOG(LogVXDataTools, Error, TEXT("BuildUI: view model class %s not found"), ClassName);
				++Errors;
				return false;
			}
			UMVVMBlueprintView* View = MVVM->RequestView(BP);
			for (const FMVVMBlueprintViewModelContext& Context : View->GetViewModels())
			{
				if (Context.GetViewModelClass() == ViewModelClass)
				{
					ViewModelId = Context.GetViewModelId();
					return true;
				}
			}
			ViewModelId = MVVM->AddViewModel(BP, ViewModelClass);
			if (FMVVMBlueprintViewModelContext* Context = View->FindViewModel(ViewModelId))
			{
				Context->CreationType = EMVVMBlueprintViewModelContextCreationType::Manual;
			}
			UE_LOG(LogVXDataTools, Display, TEXT("BuildUI: %s had no %s, added (Manual)"), *BP->GetName(), ClassName);
			return true;
		}

		template <typename T>
		T* Make(FName InName, UClass* Class = T::StaticClass())
		{
			const FName Name = ResolveName(InName);
			T* Widget = Tree()->ConstructWidget<T>(Class, Name);
			Widget->bIsVariable = true;
			BP->OnVariableAdded(Name);
			return Widget;
		}

		UCommonTextBlock* Text(FName Name, const TCHAR* Style, bool bWrap = false)
		{
			UCommonTextBlock* Widget = Make<UCommonTextBlock>(Name);
			if (UClass* StyleClass = FindGameClass(Style))
			{
				Widget->SetStyle(StyleClass);
			}
			Widget->SetJustification(ETextJustify::Center);
			Widget->SetAutoWrapText(bWrap);
			Widget->SetText(FText::FromName(Name)); // 에디터 미리보기용. 게임에서는 바인딩 값으로 바뀐다
			return Widget;
		}

		UCommonBorder* Border(FName Name, const TCHAR* Style)
		{
			UCommonBorder* Widget = Make<UCommonBorder>(Name);
			if (UClass* StyleClass = FindGameClass(Style))
			{
				Widget->SetStyle(StyleClass);
			}
			return Widget;
		}

		USizeBox* Size(FName Name, float Width, float Height)
		{
			USizeBox* Widget = Make<USizeBox>(Name);
			if (Width > 0.f)
			{
				Widget->SetWidthOverride(Width);
			}
			if (Height > 0.f)
			{
				Widget->SetHeightOverride(Height);
			}
			return Widget;
		}

		UVerticalBoxSlot* AddCentered(UVerticalBox* Box, UWidget* Child, const FMargin& Padding = FMargin(0.f))
		{
			UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
			Slot->SetHorizontalAlignment(HAlign_Center);
			Slot->SetPadding(Padding);
			return Slot;
		}

		/**
		 * 위젯 속성 ← 뷰모델 속성 (Path는 뷰모델에서 시작하는 속성 이름들. 예: {"SkillSlot0", "KeyText"})
		 */
		void Bind(FName WidgetName, UClass* WidgetClass, const TCHAR* WidgetProperty, std::initializer_list<const TCHAR*> Path)
		{
			FMVVMBlueprintPropertyPath Source;
			Source.SetViewModelId(ViewModelId);
			UClass* Current = ViewModelClass;
			bool bFirst = true;
			for (const TCHAR* Name : Path)
			{
				const FProperty* Property = nullptr != Current ? FindFProperty<FProperty>(Current, Name) : nullptr;
				if (nullptr == Property)
				{
					UE_LOG(LogVXDataTools, Error, TEXT("BuildUI: %s: view model property %s not found"), *BP->GetName(), Name);
					++Errors;
					return;
				}
				const UE::MVVM::FMVVMConstFieldVariant Field(Property);
				bFirst ? Source.SetPropertyPath(BP, Field) : Source.AppendPropertyPath(BP, Field);
				bFirst = false;
				const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
				Current = nullptr != ObjectProperty ? ObjectProperty->PropertyClass.Get() : nullptr;
			}

			const FProperty* TargetProperty = FindFProperty<FProperty>(WidgetClass, WidgetProperty);
			if (nullptr == TargetProperty)
			{
				UE_LOG(LogVXDataTools, Error, TEXT("BuildUI: %s: widget property %s.%s not found"), *BP->GetName(), *WidgetClass->GetName(), WidgetProperty);
				++Errors;
				return;
			}
			FMVVMBlueprintPropertyPath Destination;
			Destination.SetWidgetName(ResolveName(WidgetName));
			Destination.SetPropertyPath(BP, UE::MVVM::FMVVMConstFieldVariant(TargetProperty));

			FMVVMBlueprintViewBinding& Binding = MVVM->AddBinding(BP);
			MVVM->SetSourcePathForBinding(BP, Binding, Source);
			MVVM->SetDestinationPathForBinding(BP, Binding, Destination, false);
		}

		void BindText(FName WidgetName, std::initializer_list<const TCHAR*> Path)
		{
			Bind(WidgetName, UCommonTextBlock::StaticClass(), TEXT("Text"), Path);
		}
	};

	/** 버튼 WBP의 기본 스타일 (클래스 기본값) */
	void SetButtonStyle(UWidgetBlueprint* BP, const TCHAR* StyleName)
	{
		UClass* StyleClass = FindGameClass(StyleName);
		UObject* Defaults = nullptr != BP->GeneratedClass ? BP->GeneratedClass->GetDefaultObject() : nullptr;
		FClassProperty* StyleProperty = FindFProperty<FClassProperty>(UCommonButtonBase::StaticClass(), TEXT("Style"));
		if (StyleClass && Defaults && StyleProperty)
		{
			StyleProperty->SetObjectPropertyValue_InContainer(Defaults, StyleClass);
		}
	}

	bool CompileAndSave(UWidgetBlueprint* BP)
	{
		FCompilerResultsLog Results;
		FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::SkipGarbageCollection, &Results);
		for (const TSharedRef<FTokenizedMessage>& Message : Results.Messages)
		{
			if (Message->GetSeverity() == EMessageSeverity::Error || Message->GetSeverity() == EMessageSeverity::Warning)
			{
				UE_LOG(LogVXDataTools, Warning, TEXT("BuildUI: %s: %s"), *BP->GetName(), *Message->ToText().ToString());
			}
		}
		const bool bOk = BP->Status != BS_Error;
		BP->MarkPackageDirty();
		const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ BP->GetPackage() }, false);
		UE_LOG(LogVXDataTools, Display, TEXT("BuildUI: %s compiled %s, saved %d (errors %d, warnings %d)"),
			*BP->GetName(), bOk ? TEXT("OK") : TEXT("with ERRORS"), bSaved ? 1 : 0, Results.NumErrors, Results.NumWarnings);
		return bOk && bSaved;
	}

	FUIBuilder Begin(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM)
	{
		FUIBuilder Builder;
		Builder.BP = BP;
		Builder.MVVM = MVVM;
		BP->Modify();
		Builder.Reset();
		return Builder;
	}

	// -----------------------------------------------------------------------
	// 3.4 메뉴 버튼
	// -----------------------------------------------------------------------
	bool BuildMenuButton(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM)
	{
		FUIBuilder B = Begin(BP, MVVM);
		USizeBox* Root = B.Size(TEXT("Root"), 320.f, 56.f);
		B.Tree()->RootWidget = Root;
		UCommonTextBlock* Label = B.Text(TEXT("LabelText"), TEXT("VXTextStyle_Body"));
		if (USizeBoxSlot* LabelSlot = Cast<USizeBoxSlot>(Root->AddChild(Label)))
		{
			LabelSlot->SetHorizontalAlignment(HAlign_Center);
			LabelSlot->SetVerticalAlignment(VAlign_Center);
		}
		SetButtonStyle(BP, TEXT("VXButtonStyle_Menu"));
		return 0 == B.Errors && CompileAndSave(BP);
	}

	// -----------------------------------------------------------------------
	// 3.2 보상 카드
	// -----------------------------------------------------------------------
	bool BuildRewardCard(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM)
	{
		FUIBuilder B = Begin(BP, MVVM);
		if (false == B.UseViewModel(TEXT("VX_VM_RewardCard")))
		{
			return false;
		}
		USizeBox* Root = B.Size(TEXT("Root"), 280.f, 340.f);
		B.Tree()->RootWidget = Root;
		UVerticalBox* Box = B.Make<UVerticalBox>(TEXT("CardBox"));
		Root->AddChild(Box);
		B.AddCentered(Box, B.Text(TEXT("SkillNameText"), TEXT("VXTextStyle_Title")), FMargin(0, 8, 0, 4));
		B.AddCentered(Box, B.Text(TEXT("ModifierNameText"), TEXT("VXTextStyle_Accent")), FMargin(0, 0, 0, 4));
		B.AddCentered(Box, B.Text(TEXT("LevelText"), TEXT("VXTextStyle_Muted")), FMargin(0, 0, 0, 16));
		UVerticalBoxSlot* DescSlot = B.AddCentered(Box, B.Text(TEXT("DescriptionText"), TEXT("VXTextStyle_Body"), true), FMargin(0, 0, 0, 16));
		DescSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		B.AddCentered(Box, B.Text(TEXT("TagText"), TEXT("VXTextStyle_Accent")));

		B.BindText(TEXT("SkillNameText"), { TEXT("SkillName") });
		B.BindText(TEXT("ModifierNameText"), { TEXT("ModifierName") });
		B.BindText(TEXT("LevelText"), { TEXT("LevelText") });
		B.BindText(TEXT("DescriptionText"), { TEXT("Description") });
		B.BindText(TEXT("TagText"), { TEXT("TagText") });
		SetButtonStyle(BP, TEXT("VXButtonStyle_Card"));
		return 0 == B.Errors && CompileAndSave(BP);
	}

	// -----------------------------------------------------------------------
	// 3.1 HUD
	// -----------------------------------------------------------------------
	bool BuildHUD(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM)
	{
		FUIBuilder B = Begin(BP, MVVM);
		if (false == B.UseViewModel(TEXT("VX_VM_Hud")))
		{
			return false;
		}
		UCanvasPanel* Root = B.Make<UCanvasPanel>(TEXT("Root"));
		Root->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		B.Tree()->RootWidget = Root;

		auto Place = [Root](UWidget* Widget, FVector2D Anchor, FVector2D Position)
		{
			UCanvasPanelSlot* Slot = Root->AddChildToCanvas(Widget);
			Slot->SetAnchors(FAnchors(Anchor.X, Anchor.Y));
			Slot->SetAlignment(Anchor);
			Slot->SetAutoSize(true);
			Slot->SetPosition(Position);
		};

		// 상단: 웨이브
		UVerticalBox* Top = B.Make<UVerticalBox>(TEXT("WaveBox"));
		Place(Top, FVector2D(0.5f, 0.f), FVector2D(0.f, 24.f));
		B.AddCentered(Top, B.Text(TEXT("WaveText"), TEXT("VXTextStyle_Title")));
		B.AddCentered(Top, B.Text(TEXT("EnemiesText"), TEXT("VXTextStyle_Body")));

		// 배너
		UVerticalBox* Banner = B.Make<UVerticalBox>(TEXT("BannerBox"));
		Banner->SetVisibility(ESlateVisibility::HitTestInvisible);
		Banner->SetRenderOpacity(0.f);
		Place(Banner, FVector2D(0.5f, 0.5f), FVector2D(0.f, -220.f));
		B.AddCentered(Banner, B.Text(TEXT("BannerText"), TEXT("VXTextStyle_Banner")));
		B.AddCentered(Banner, B.Text(TEXT("BannerSubText"), TEXT("VXTextStyle_Accent")));

		// 좌측 하단: 체력
		UVerticalBox* Health = B.Make<UVerticalBox>(TEXT("HealthBox"));
		Place(Health, FVector2D(0.f, 1.f), FVector2D(40.f, -40.f));
		Health->AddChildToVerticalBox(B.Text(TEXT("HealthText"), TEXT("VXTextStyle_Body")))->SetPadding(FMargin(0, 0, 0, 6));
		USizeBox* HealthSize = B.Size(TEXT("HealthBarSize"), 320.f, 18.f);
		UProgressBar* HealthBar = B.Make<UProgressBar>(TEXT("HealthBar"));
		HealthBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		HealthBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.85f, 0.35f));
		HealthSize->AddChild(HealthBar);
		Health->AddChildToVerticalBox(HealthSize);

		// 하단 중앙: 스킬 슬롯 4개
		UHorizontalBox* Row = B.Make<UHorizontalBox>(TEXT("SkillRow"));
		Place(Row, FVector2D(0.5f, 1.f), FVector2D(0.f, -32.f));
		for (int32 i = 0; i < 4; ++i)
		{
			const FString P = FString::Printf(TEXT("Slot%d_"), i);
			USizeBox* SlotSize = B.Size(*(P + TEXT("Size")), 170.f, 0.f);
			UHorizontalBoxSlot* RowSlot = Row->AddChildToHorizontalBox(SlotSize);
			RowSlot->SetPadding(FMargin(8, 0));
			RowSlot->SetVerticalAlignment(VAlign_Bottom);

			UCommonBorder* Panel = B.Border(*(P + TEXT("Panel")), TEXT("VXBorderStyle_Panel"));
			Panel->SetPadding(FMargin(10));
			SlotSize->AddChild(Panel);
			UVerticalBox* Box = B.Make<UVerticalBox>(*(P + TEXT("Box")));
			Panel->AddChild(Box);

			B.AddCentered(Box, B.Text(*(P + TEXT("Key")), TEXT("VXTextStyle_Muted")));
			B.AddCentered(Box, B.Text(*(P + TEXT("Name")), TEXT("VXTextStyle_Body")));
			USizeBox* BarSize = B.Size(*(P + TEXT("CooldownSize")), 0.f, 6.f);
			UProgressBar* Bar = B.Make<UProgressBar>(*(P + TEXT("CooldownBar")));
			Bar->SetVisibility(ESlateVisibility::HitTestInvisible);
			Bar->SetFillColorAndOpacity(AccentColor);
			BarSize->AddChild(Bar);
			Box->AddChildToVerticalBox(BarSize)->SetPadding(FMargin(0, 6, 0, 2));
			B.AddCentered(Box, B.Text(*(P + TEXT("CooldownText")), TEXT("VXTextStyle_Accent")));
			B.AddCentered(Box, B.Text(*(P + TEXT("Modifiers")), TEXT("VXTextStyle_Accent"), true));

			const FString SlotName = FString::Printf(TEXT("SkillSlot%d"), i);
			B.BindText(*(P + TEXT("Key")), { *SlotName, TEXT("KeyText") });
			B.BindText(*(P + TEXT("Name")), { *SlotName, TEXT("SkillName") });
			B.Bind(*(P + TEXT("CooldownBar")), UProgressBar::StaticClass(), TEXT("Percent"), { *SlotName, TEXT("CooldownPercent") });
			B.BindText(*(P + TEXT("CooldownText")), { *SlotName, TEXT("CooldownText") });
			B.BindText(*(P + TEXT("Modifiers")), { *SlotName, TEXT("ModifiersText") });
		}

		B.BindText(TEXT("WaveText"), { TEXT("WaveText") });
		B.BindText(TEXT("EnemiesText"), { TEXT("EnemiesText") });
		B.BindText(TEXT("BannerText"), { TEXT("BannerText") });
		B.BindText(TEXT("BannerSubText"), { TEXT("BannerSubText") });
		B.Bind(TEXT("BannerBox"), UWidget::StaticClass(), TEXT("RenderOpacity"), { TEXT("BannerOpacity") });
		B.BindText(TEXT("HealthText"), { TEXT("HealthText") });
		B.Bind(TEXT("HealthBar"), UProgressBar::StaticClass(), TEXT("Percent"), { TEXT("HealthPercent") });
		return 0 == B.Errors && CompileAndSave(BP);
	}

	/** 메뉴 화면 공통: 어둡게 + 가운데 세로 상자 */
	UVerticalBox* BuildMenuFrame(FUIBuilder& B)
	{
		UOverlay* Root = B.Make<UOverlay>(TEXT("Root"));
		B.Tree()->RootWidget = Root;
		UCommonBorder* Dim = B.Border(TEXT("Dim"), TEXT("VXBorderStyle_Dim"));
		UOverlaySlot* DimSlot = Root->AddChildToOverlay(Dim);
		DimSlot->SetHorizontalAlignment(HAlign_Fill);
		DimSlot->SetVerticalAlignment(VAlign_Fill);
		UVerticalBox* Box = B.Make<UVerticalBox>(TEXT("Content"));
		UOverlaySlot* BoxSlot = Root->AddChildToOverlay(Box);
		BoxSlot->SetHorizontalAlignment(HAlign_Center);
		BoxSlot->SetVerticalAlignment(VAlign_Center);
		return Box;
	}

	// -----------------------------------------------------------------------
	// 3.3 보상 선택
	// -----------------------------------------------------------------------
	bool BuildRewardSelect(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM, UClass* CardClass)
	{
		FUIBuilder B = Begin(BP, MVVM);
		if (false == B.UseViewModel(TEXT("VX_VM_Reward")))
		{
			return false;
		}
		UVerticalBox* Box = BuildMenuFrame(B);
		B.AddCentered(Box, B.Text(TEXT("TitleText"), TEXT("VXTextStyle_Title")), FMargin(0, 0, 0, 32));
		UHorizontalBox* Cards = B.Make<UHorizontalBox>(TEXT("CardRow"));
		B.AddCentered(Box, Cards);
		for (const TCHAR* Name : { TEXT("Card0"), TEXT("Card1"), TEXT("Card2") })
		{
			UUserWidget* Card = B.Make<UUserWidget>(Name, CardClass);
			UHorizontalBoxSlot* CardSlot = Cards->AddChildToHorizontalBox(Card);
			CardSlot->SetPadding(FMargin(20, 0));
			CardSlot->SetVerticalAlignment(VAlign_Center);
		}
		B.AddCentered(Box, B.Text(TEXT("BuildText"), TEXT("VXTextStyle_Accent"), true), FMargin(0, 40, 0, 8));
		B.AddCentered(Box, B.Text(TEXT("HintText"), TEXT("VXTextStyle_Muted")), FMargin(0, 8, 0, 0));

		B.BindText(TEXT("TitleText"), { TEXT("TitleText") });
		B.BindText(TEXT("BuildText"), { TEXT("BuildText") });
		B.BindText(TEXT("HintText"), { TEXT("HintText") });
		return 0 == B.Errors && CompileAndSave(BP);
	}

	// -----------------------------------------------------------------------
	// 3.5 일시정지 / 3.6 결과
	// -----------------------------------------------------------------------
	void AddMenuButtons(FUIBuilder& B, UVerticalBox* Box, UClass* ButtonClass, std::initializer_list<const TCHAR*> Names)
	{
		for (const TCHAR* Name : Names)
		{
			UUserWidget* Button = B.Make<UUserWidget>(Name, ButtonClass);
			B.AddCentered(Box, Button, FMargin(0, 8));
		}
	}

	bool BuildPause(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM, UClass* ButtonClass)
	{
		FUIBuilder B = Begin(BP, MVVM);
		if (false == B.UseViewModel(TEXT("VX_VM_Pause")))
		{
			return false;
		}
		UVerticalBox* Box = BuildMenuFrame(B);
		B.AddCentered(Box, B.Text(TEXT("TitleText"), TEXT("VXTextStyle_Title")), FMargin(0, 0, 0, 32));
		AddMenuButtons(B, Box, ButtonClass, { TEXT("ResumeButton"), TEXT("RestartButton"), TEXT("QuitButton") });
		B.BindText(TEXT("TitleText"), { TEXT("TitleText") });
		return 0 == B.Errors && CompileAndSave(BP);
	}

	bool BuildResult(UWidgetBlueprint* BP, UMVVMEditorSubsystem* MVVM, UClass* ButtonClass)
	{
		FUIBuilder B = Begin(BP, MVVM);
		if (false == B.UseViewModel(TEXT("VX_VM_Result")))
		{
			return false;
		}
		UVerticalBox* Box = BuildMenuFrame(B);
		B.AddCentered(Box, B.Text(TEXT("TitleText"), TEXT("VXTextStyle_Banner")), FMargin(0, 0, 0, 16));
		B.AddCentered(Box, B.Text(TEXT("StatsText"), TEXT("VXTextStyle_Body"), true), FMargin(0, 0, 0, 16));
		B.AddCentered(Box, B.Text(TEXT("BuildText"), TEXT("VXTextStyle_Accent"), true), FMargin(0, 0, 0, 32));
		AddMenuButtons(B, Box, ButtonClass, { TEXT("RestartButton"), TEXT("QuitButton") });
		B.BindText(TEXT("TitleText"), { TEXT("TitleText") });
		B.BindText(TEXT("StatsText"), { TEXT("StatsText") });
		B.BindText(TEXT("BuildText"), { TEXT("BuildText") });
		return 0 == B.Errors && CompileAndSave(BP);
	}
}

UVXBuildUICommandlet::UVXBuildUICommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
}

int32 UVXBuildUICommandlet::Main(const FString& Params)
{
	UMVVMEditorSubsystem* MVVM = nullptr != GEditor ? GEditor->GetEditorSubsystem<UMVVMEditorSubsystem>() : nullptr;
	if (nullptr == MVVM)
	{
		MVVM = NewObject<UMVVMEditorSubsystem>();
	}

	UWidgetBlueprint* MenuButton = LoadWBP(TEXT("WBP_VX_MenuButton"));
	UWidgetBlueprint* RewardCard = LoadWBP(TEXT("WBP_VX_RewardCard"));
	UWidgetBlueprint* HUD = LoadWBP(TEXT("WBP_VX_HUD"));
	UWidgetBlueprint* RewardSelect = LoadWBP(TEXT("WBP_VX_RewardSelect"));
	UWidgetBlueprint* Pause = LoadWBP(TEXT("WBP_VX_Pause"));
	UWidgetBlueprint* Result = LoadWBP(TEXT("WBP_VX_Result"));
	if (nullptr == MenuButton || nullptr == RewardCard || nullptr == HUD || nullptr == RewardSelect || nullptr == Pause || nullptr == Result)
	{
		UE_LOG(LogVXDataTools, Error, TEXT("BuildUI: some WBP_VX_* assets are missing in /Game/Voxelcaster/UI"));
		return 1;
	}

	// 부품을 먼저 (다른 화면이 부품 클래스를 배치한다)
	int32 Failed = 0;
	Failed += BuildMenuButton(MenuButton, MVVM) ? 0 : 1;
	Failed += BuildRewardCard(RewardCard, MVVM) ? 0 : 1;
	Failed += BuildHUD(HUD, MVVM) ? 0 : 1;
	Failed += BuildRewardSelect(RewardSelect, MVVM, RewardCard->GeneratedClass) ? 0 : 1;
	Failed += BuildPause(Pause, MVVM, MenuButton->GeneratedClass) ? 0 : 1;
	Failed += BuildResult(Result, MVVM, MenuButton->GeneratedClass) ? 0 : 1;

	UE_LOG(LogVXDataTools, Display, TEXT("BuildUI: done, %d failed"), Failed);
	return 0 == Failed ? 0 : 1;
}
