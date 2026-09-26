// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "VXPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UCommonActivatableWidget;
class UVXHUDWidget;
class UVX_VM_Hud;
class UVXPauseWidget;
class UVXResultWidget;
class UVXRewardSelectWidget;
struct FInputActionValue;

/** 마지막으로 입력이 들어온 장치. 게임패드는 Xbox·PlayStation 등 콘솔 컨트롤러를 모두 포함한다. */
UENUM(BlueprintType)
enum class EVXInputDevice : uint8
{
	KeyboardMouse,
	Gamepad
};

DECLARE_MULTICAST_DELEGATE_OneParam(FVXInputDeviceChangedSignature, EVXInputDevice /*NewDevice*/);

/**
 * 게임플레이 입력 담당 (DES-CTRL-001).
 * - Enhanced Input 액션과 매핑 컨텍스트를 코드로 만든다. (에셋 없이 동작, 추후 에셋으로 옮길 수 있다)
 * - 스킬·대시 입력은 입력 태그(Input.*)로 ASC에 전달한다.
 * - 조준: 키보드는 마우스 위치, 게임패드는 오른쪽 스틱(없으면 이동 방향).
 * - UI 입력은 CommonUI가 담당한다. (메뉴 화면 작업 시 연결)
 */
UCLASS()
class VOXELCASTER_API AVXPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVXPlayerController();

	/** 보상 카드 추첨 결과를 받아 보상 선택 화면을 연다 (치트에서도 호출) */
	void OpenRewardSelect(const TArray<struct FVXUpgradeCard>& Choices);

	/** 메뉴 화면(일시정지·결과)을 띄운다: 게임 정지, UI 입력, 기본 포커스 */
	void OpenMenu(UCommonActivatableWidget* Menu);
	/** 열린 메뉴를 닫고 게임을 재개한다 */
	void CloseMenu();
	bool IsMenuOpen() const { return nullptr != CurrentMenu; }

	/** 현재 레벨을 다시 연다 (웨이브·체력·빌드·처치 수 모두 초기화) */
	void RestartGame();
	void QuitGame();

	EVXInputDevice GetInputDevice() const { return InputDevice; }

	/** 입력 장치가 바뀔 때 (HUD 버튼 아이콘, 커서 표시 갱신용) */
	FVXInputDeviceChangedSignature OnInputDeviceChanged;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

private:
	void CreateInputAssets();
	void BindInputActions();

	void HandleMove(const FInputActionValue& Value);
	void HandleMoveCompleted(const FInputActionValue& Value);
	void HandleAim(const FInputActionValue& Value);
	void HandleAimCompleted(const FInputActionValue& Value);
	void HandleAbilityPressed(FGameplayTag InputTag);
	void HandleAbilityReleased(FGameplayTag InputTag);
	void HandlePause(const FInputActionValue& Value);
	void HandleGameEnded(bool bVictory);
	void ShowResult(bool bVictory);
	void UpdateHudViewModel();

	void SetInputDevice(EVXInputDevice NewDevice);
	void HandleChoicesReady(const TArray<struct FVXUpgradeCard>& Choices);
	/** 화면 좌측 상단에 플레이어 HP를 표시한다 (HUD가 생기면 끈다) */
	void ShowDebugInfo() const;
	FVector ResolveAimDirection() const;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> GameplayContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY()
	TObjectPtr<UInputAction> AimAction;
	UPROPERTY()
	TObjectPtr<UInputAction> Skill1Action;
	UPROPERTY()
	TObjectPtr<UInputAction> Skill2Action;
	UPROPERTY()
	TObjectPtr<UInputAction> Skill3Action;
	UPROPERTY()
	TObjectPtr<UInputAction> DashAction;
	UPROPERTY()
	TObjectPtr<UInputAction> PauseAction;

	/**
	 * 화면 클래스. WBP를 만들면 여기로 연결된다 (기본 경로 /Game/Voxelcaster/UI/WBP_VX_*).
	 * WBP가 없으면 C++ 기본 화면을 쓴다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|UI")
	TSoftClassPtr<UVXHUDWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|UI")
	TSoftClassPtr<UVXRewardSelectWidget> RewardSelectWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|UI")
	TSoftClassPtr<UVXPauseWidget> PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|UI")
	TSoftClassPtr<UVXResultWidget> ResultWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UVXHUDWidget> HUDWidget;

	/** HUD 뷰모델. 매 프레임 게임 상태로 채운다 (바뀐 값만 WBP에 알려진다) */
	UPROPERTY(Transient)
	TObjectPtr<UVX_VM_Hud> HudViewModel;

	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> CurrentMenu;

	FTimerHandle ResultTimer;

	/** 화면 좌측 상단에 플레이어 HP를 표시한다 (HUD가 생기면 끈다) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Debug")
	bool bShowDebugInfo = false;

	EVXInputDevice InputDevice = EVXInputDevice::KeyboardMouse;

	/** 오른쪽 스틱 조준 값 (월드 방향, 입력 없으면 Zero) */
	FVector StickAimDirection = FVector::ZeroVector;

	FVector2D LastMousePosition = FVector2D::ZeroVector;
	bool bHasLastMousePosition = false;

	/** 마우스가 이 픽셀 이상 움직여야 키보드·마우스로 전환한다 (노이즈 방지) */
	static constexpr float MouseSwitchThreshold = 5.f;
	/** 게임패드 축 입력이 이 값 이상이어야 게임패드로 전환한다 (스틱 드리프트 방지) */
	static constexpr float GamepadSwitchThreshold = 0.25f;
};
