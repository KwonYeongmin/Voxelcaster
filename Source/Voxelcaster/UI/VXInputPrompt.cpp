// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXInputPrompt.h"
#include "CommonInputSubsystem.h"
#include "Data/VXDataManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "HAL/IConsoleManager.h"
#include "Player/VXPlayerController.h"
#include "UI/VXText.h"

namespace
{
	TAutoConsoleVariable<FString> CVarGamepadStyle(
		TEXT("VX.GamepadStyle"), TEXT("auto"),
		TEXT("게임패드 버튼 표기: auto / xbox / ps"));

	struct FPromptLabels
	{
		const TCHAR* Keyboard;
		const TCHAR* Xbox;
		const TCHAR* PlayStation;
	};

	/** EVXPromptAction 순서. 입력 매핑은 AVXPlayerController::CreateInputAssets */
	const FPromptLabels Labels[] =
	{
		{ TEXT("LMB"),   TEXT("RT"),    TEXT("R2") },       // MagicBolt
		{ TEXT("RMB"),   TEXT("LT"),    TEXT("L2") },       // Nova
		{ TEXT("Q"),     TEXT("RB"),    TEXT("R1") },       // BladeSweep
		{ TEXT("Space"), TEXT("A"),     TEXT("×") },        // Dash
		{ TEXT("Enter"), TEXT("A"),     TEXT("×") },        // Confirm
		{ TEXT("← →"),   TEXT("D-Pad"), TEXT("D-Pad") },    // Navigate
		{ TEXT("Esc"),   TEXT("B"),     TEXT("O") },        // Back
		{ TEXT("Esc"),   TEXT("Menu"),  TEXT("Options") },  // Pause
	};
}

namespace VXInputPrompt
{
	bool IsGamepad(const APlayerController* PlayerController)
	{
		const AVXPlayerController* VXController = Cast<AVXPlayerController>(PlayerController);
		return nullptr != VXController && EVXInputDevice::Gamepad == VXController->GetInputDevice();
	}

	EVXGamepadStyle GetGamepadStyle(const APlayerController* PlayerController)
	{
		const FString Style = CVarGamepadStyle.GetValueOnGameThread();
		if (Style.Equals(TEXT("ps"), ESearchCase::IgnoreCase))
		{
			return EVXGamepadStyle::PlayStation;
		}
		if (Style.Equals(TEXT("xbox"), ESearchCase::IgnoreCase))
		{
			return EVXGamepadStyle::Xbox;
		}

		// auto: CommonInput이 감지한 패드 이름 (플랫폼·드라이버에 따라 Generic일 수 있다 → Xbox 표기)
		const ULocalPlayer* LocalPlayer = nullptr != PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
		if (const UCommonInputSubsystem* CommonInput = UCommonInputSubsystem::Get(LocalPlayer))
		{
			const FString Name = CommonInput->GetCurrentGamepadName().ToString();
			if (Name.Contains(TEXT("PS")) || Name.Contains(TEXT("DualSense")) || Name.Contains(TEXT("DualShock")) || Name.Contains(TEXT("PlayStation")))
			{
				return EVXGamepadStyle::PlayStation;
			}
		}
		return EVXGamepadStyle::Xbox;
	}

	FString GetLabel(EVXPromptAction Action, const APlayerController* PlayerController)
	{
		const int32 Index = static_cast<int32>(Action);
		if (Index < 0 || Index >= UE_ARRAY_COUNT(Labels))
		{
			return FString();
		}

		const FPromptLabels& Label = Labels[Index];
		if (false == IsGamepad(PlayerController))
		{
			// 확인은 클릭도 된다
			return EVXPromptAction::Confirm == Action
				? FString::Printf(TEXT("%s / %s"), Label.Keyboard, *VXText::Get(TEXT("Key.Click")))
				: FString(Label.Keyboard);
		}
		return EVXGamepadStyle::PlayStation == GetGamepadStyle(PlayerController) ? Label.PlayStation : Label.Xbox;
	}

	UTexture2D* GetIcon(EVXPromptAction Action, const APlayerController* PlayerController)
	{
		UVXDataManager* Data = UVXDataManager::Get();
		const UDataTable* Table = nullptr != Data ? Data->GetTable(EVXDataTable::InputIcons) : nullptr;
		if (nullptr == Table)
		{
			return nullptr;
		}

		const FString RowName = StaticEnum<EVXPromptAction>()->GetNameStringByValue(static_cast<int64>(Action));
		const FVXInputIconRow* Row = Table->FindRow<FVXInputIconRow>(*RowName, TEXT("VXInputPrompt"), false);
		if (nullptr == Row)
		{
			return nullptr;
		}

		const TSoftObjectPtr<UTexture2D>& Icon = false == IsGamepad(PlayerController) ? Row->Keyboard
			: (EVXGamepadStyle::PlayStation == GetGamepadStyle(PlayerController) ? Row->PlayStation : Row->Xbox);
		return Icon.IsNull() ? nullptr : Icon.LoadSynchronous();
	}
}
