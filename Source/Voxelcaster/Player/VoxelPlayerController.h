// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "VoxelPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/** 마지막으로 입력이 들어온 장치. 게임패드는 Xbox·PlayStation 등 콘솔 컨트롤러를 모두 포함한다. */
UENUM(BlueprintType)
enum class EVoxelInputDevice : uint8
{
	KeyboardMouse,
	Gamepad
};

DECLARE_MULTICAST_DELEGATE_OneParam(FVXInputDeviceChangedSignature, EVoxelInputDevice /*NewDevice*/);

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

	EVoxelInputDevice GetInputDevice() const { return InputDevice; }

	/** 입력 장치가 바뀔 때 (HUD 버튼 아이콘, 커서 표시 갱신용) */
	FVXInputDeviceChangedSignature OnInputDeviceChanged;

	// ---- 디버그 콘솔 명령 ----
	UFUNCTION(Exec)
	void DebugDamage(float Amount = 10.f);

	UFUNCTION(Exec)
	void DebugHeal(float Amount = 30.f);

	UFUNCTION(Exec)
	void DebugKill();

	/** 플레이어 주변 링(8~12m)에 러너를 소환한다. 스킬 테스트용. */
	UFUNCTION(Exec)
	void DebugSpawnRunners(int32 Count = 10);

	/** 지정한 웨이브(1~5)를 시작한다. 인자를 생략하면 1 */
	UFUNCTION(Exec)
	void DebugStartWave(int32 WaveIndex = 1);

	/** 웨이브 진행을 멈춘다. */
	UFUNCTION(Exec)
	void DebugStopWaves();

	/**
	 * 지정한 적을 플레이어 주변(8~12m)에 소환한다. 웨이브와 무관한 테스트용.
	 * 예) DebugSpawnEnemy Elite 1 / DebugSpawnEnemy Shooter 3
	 */
	UFUNCTION(Exec)
	void DebugSpawnEnemy(const FString& EnemyType, int32 Count = 1);

	/** 살아 있는 모든 적에게 피해를 준다. 체력 확인용. 예) DebugDamageEnemies 20 */
	UFUNCTION(Exec)
	void DebugDamageEnemies(float Amount = 20.f);

	/** 살아 있는 모든 적을 제거한다. */
	UFUNCTION(Exec)
	void DebugKillEnemies();

protected:
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

	void SetInputDevice(EVoxelInputDevice NewDevice);
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

	EVoxelInputDevice InputDevice = EVoxelInputDevice::KeyboardMouse;

	/** 오른쪽 스틱 조준 값 (월드 방향, 입력 없으면 Zero) */
	FVector StickAimDirection = FVector::ZeroVector;

	FVector2D LastMousePosition = FVector2D::ZeroVector;
	bool bHasLastMousePosition = false;

	/** 마우스가 이 픽셀 이상 움직여야 키보드·마우스로 전환한다 (노이즈 방지) */
	static constexpr float MouseSwitchThreshold = 5.f;
	/** 게임패드 축 입력이 이 값 이상이어야 게임패드로 전환한다 (스틱 드리프트 방지) */
	static constexpr float GamepadSwitchThreshold = 0.25f;
};
