// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VXInputPrompt.generated.h"

class APlayerController;
class UTexture2D;

/** 화면에 안내하는 입력 동작. DT_InputIcons의 행 이름과 같다 */
UENUM(BlueprintType)
enum class EVXPromptAction : uint8
{
	MagicBolt,
	Nova,
	BladeSweep,
	Dash,
	/** UI 확인 (카드 선택, 버튼) */
	Confirm,
	/** UI 좌우 이동 */
	Navigate,
	/** UI 뒤로 */
	Back,
	Pause
};

/** 게임패드 버튼 표기 방식 */
UENUM(BlueprintType)
enum class EVXGamepadStyle : uint8
{
	Xbox,
	PlayStation
};

/**
 * 입력 아이콘 한 줄. DT_InputIcons의 행 구조체다. 행 이름은 EVXPromptAction 이름 (MagicBolt, Confirm ...).
 * 비워 둔 칸은 아이콘 없이 글자(LMB, RT, R2 ...)로 표시한다.
 */
USTRUCT(BlueprintType)
struct FVXInputIconRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Keyboard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Xbox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> PlayStation;
};

/**
 * 입력 안내 (DES-CTRL-001). 마지막으로 쓴 입력 장치에 맞는 키 글자와 아이콘을 돌려준다.
 * - 장치: AVXPlayerController::GetInputDevice (게임플레이 입력 + CommonUI 입력 감지)
 * - 게임패드 표기: 콘솔 변수 VX.GamepadStyle (auto / xbox / ps). auto는 CommonInput이 알려 주는 패드 이름으로 고른다
 * - 아이콘: 데이터 매니저의 InputIcons 테이블 (없어도 된다)
 */
namespace VXInputPrompt
{
	VOXELCASTER_API bool IsGamepad(const APlayerController* PlayerController);
	VOXELCASTER_API EVXGamepadStyle GetGamepadStyle(const APlayerController* PlayerController);

	/** 키 글자 (LMB / RT / R2 ...) */
	VOXELCASTER_API FString GetLabel(EVXPromptAction Action, const APlayerController* PlayerController);

	/** 아이콘. 테이블이나 칸이 비어 있으면 nullptr */
	VOXELCASTER_API UTexture2D* GetIcon(EVXPromptAction Action, const APlayerController* PlayerController);
}
