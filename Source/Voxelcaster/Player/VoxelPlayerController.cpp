// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/VoxelPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Character/VoxelCharacterBase.h"
#include "Core/VoxelGameMode.h"
#include "Enemy/VoxelElite.h"
#include "Enemy/VoxelRunner.h"
#include "Enemy/VoxelShooter.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/VoxelAbilitySystemComponent.h"
#include "GAS/VoxelAttributeSet.h"
#include "GAS/VoxelGameplayEffects.h"
#include "GAS/VoxelGameplayTags.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Cheat/VXCheatManager.h"
#include "Modifier/VXModifierComponent.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "UI/VXRewardSelectWidget.h"
#include "Wave/VoxelWaveManager.h"
#include "Voxelcaster.h"

AVXPlayerController::AVXPlayerController()
{
	CheatClass = UVXCheatManager::StaticClass();

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

// ---------------------------------------------------------------------------
// 입력 자산 생성 (코드)
// ---------------------------------------------------------------------------

void AVXPlayerController::CreateInputAssets()
{
	const auto MakeAction = [this](const TCHAR* Name, EInputActionValueType Type)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = Type;
		return Action;
	};

	MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	AimAction = MakeAction(TEXT("IA_Aim"), EInputActionValueType::Axis2D);
	Skill1Action = MakeAction(TEXT("IA_Skill1"), EInputActionValueType::Boolean);
	Skill2Action = MakeAction(TEXT("IA_Skill2"), EInputActionValueType::Boolean);
	Skill3Action = MakeAction(TEXT("IA_Skill3"), EInputActionValueType::Boolean);
	DashAction = MakeAction(TEXT("IA_Dash"), EInputActionValueType::Boolean);

	GameplayContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Gameplay"));

	const auto AddDeadZone = [this](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierDeadZone* DeadZone = NewObject<UInputModifierDeadZone>(this);
		DeadZone->LowerThreshold = 0.25f;
		Mapping.Modifiers.Add(DeadZone);
	};

	const auto AddSwizzle = [this](FEnhancedActionKeyMapping& Mapping)
	{
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(this);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
	};

	const auto AddNegate = [this](FEnhancedActionKeyMapping& Mapping)
	{
		Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(this));
	};

	// 이동: 입력 (X = 오른쪽, Y = 앞)
	// 키보드: W(앞) / S(뒤) / A(왼쪽) / D(오른쪽)
	AddSwizzle(GameplayContext->MapKey(MoveAction, EKeys::W));
	{
		FEnhancedActionKeyMapping& S = GameplayContext->MapKey(MoveAction, EKeys::S);
		AddSwizzle(S);
		AddNegate(S);
	}
	AddNegate(GameplayContext->MapKey(MoveAction, EKeys::A));
	GameplayContext->MapKey(MoveAction, EKeys::D);
	// 게임패드: 왼쪽 스틱
	AddDeadZone(GameplayContext->MapKey(MoveAction, EKeys::Gamepad_Left2D));

	// 조준: 게임패드 오른쪽 스틱 (마우스는 위치로 직접 계산)
	AddDeadZone(GameplayContext->MapKey(AimAction, EKeys::Gamepad_Right2D));

	// 스킬 1/2/3, 대시. 게임패드 키는 Xbox·PlayStation 모두 같은 키로 매핑된다.
	// (Xbox RT/LT/RB/A = PlayStation R2/L2/R1/Cross)
	GameplayContext->MapKey(Skill1Action, EKeys::LeftMouseButton);
	GameplayContext->MapKey(Skill1Action, EKeys::Gamepad_RightTrigger);
	GameplayContext->MapKey(Skill2Action, EKeys::RightMouseButton);
	GameplayContext->MapKey(Skill2Action, EKeys::Gamepad_LeftTrigger);
	GameplayContext->MapKey(Skill3Action, EKeys::Q);
	GameplayContext->MapKey(Skill3Action, EKeys::Gamepad_RightShoulder);
	GameplayContext->MapKey(DashAction, EKeys::SpaceBar);
	GameplayContext->MapKey(DashAction, EKeys::Gamepad_FaceButton_Bottom);
}

void AVXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UVXUpgradeSubsystem* Upgrades = GetWorld()->GetSubsystem<UVXUpgradeSubsystem>())
	{
		Upgrades->OnChoicesReady.AddUObject(this, &AVXPlayerController::HandleChoicesReady);
	}
}

void AVXPlayerController::HandleChoicesReady(const TArray<FVXUpgradeCard>& Choices)
{
	OpenRewardSelect(Choices);
}

void AVXPlayerController::OpenRewardSelect(const TArray<FVXUpgradeCard>& Choices)
{
	UVXRewardSelectWidget* Widget = CreateWidget<UVXRewardSelectWidget>(this, UVXRewardSelectWidget::StaticClass());
	if (nullptr == Widget)
	{
		return;
	}

	int32 WaveIndex = 0;
	if (const AVoxelGameMode* GameMode = GetWorld()->GetAuthGameMode<AVoxelGameMode>())
	{
		WaveIndex = GameMode->GetWaveManager()->GetCurrentWave();
	}

	// 보상 선택 중에는 게임을 멈춘다. (DES-RULES-001)
	Widget->SetChoices(Choices, WaveIndex);
	Widget->AddToViewport(100);
	Widget->ActivateWidget();
	SetPause(true);

	// 포커스는 화면 루트가 아니라 가운데 카드에 준다. (루트는 포커스를 받을 수 없음)
	SetInputMode(FInputModeUIOnly());
	Widget->FocusDefaultCard();
}

void AVXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	CreateInputAssets();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(GameplayContext, 0);
		}
	}

	BindInputActions();
}

void AVXPlayerController::BindInputActions()
{
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (false == ensureMsgf(EnhancedInput, TEXT("EnhancedInputComponent가 필요합니다. DefaultInput.ini의 DefaultInputComponentClass를 확인하세요.")))
	{
		return;
	}

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVXPlayerController::HandleMove);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AVXPlayerController::HandleMoveCompleted);
	EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &AVXPlayerController::HandleAim);
	EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &AVXPlayerController::HandleAimCompleted);

	const auto BindAbility = [this, EnhancedInput](UInputAction* Action, const FGameplayTag& Tag)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &AVXPlayerController::HandleAbilityPressed, Tag);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &AVXPlayerController::HandleAbilityReleased, Tag);
	};

	BindAbility(Skill1Action, VoxelTags::Input_Skill1);
	BindAbility(Skill2Action, VoxelTags::Input_Skill2);
	BindAbility(Skill3Action, VoxelTags::Input_Skill3);
	BindAbility(DashAction, VoxelTags::Input_Dash);
}

// ---------------------------------------------------------------------------
// 입력 처리
// ---------------------------------------------------------------------------

void AVXPlayerController::HandleMove(const FInputActionValue& Value)
{
	AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == VoxelChar || VoxelChar->IsDead())
	{
		return;
	}

	// 카메라가 +X 방향을 보므로 화면 위 = 월드 +X, 화면 오른쪽 = 월드 +Y
	const FVector2D Input = Value.Get<FVector2D>();
	const FVector Direction(Input.Y, Input.X, 0.f);
	const float Magnitude = FMath::Min(Direction.Size(), 1.f);
	if (Magnitude <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const FVector Normalized = Direction.GetSafeNormal();
	VoxelChar->AddMovementInput(Normalized, Magnitude);
	VoxelChar->SetMoveInputDirection(Normalized);
}

void AVXPlayerController::HandleMoveCompleted(const FInputActionValue& Value)
{
	if (AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		VoxelChar->SetMoveInputDirection(FVector::ZeroVector);
	}
}

void AVXPlayerController::HandleAim(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	StickAimDirection = FVector(Input.Y, Input.X, 0.f).GetSafeNormal();
}

void AVXPlayerController::HandleAimCompleted(const FInputActionValue& Value)
{
	StickAimDirection = FVector::ZeroVector;
}

void AVXPlayerController::HandleAbilityPressed(FGameplayTag InputTag)
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		if (UVoxelAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void AVXPlayerController::HandleAbilityReleased(FGameplayTag InputTag)
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		if (UVoxelAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
		{
			ASC->AbilityInputTagReleased(InputTag);
		}
	}
}

// ---------------------------------------------------------------------------
// 입력 장치 감지
// ---------------------------------------------------------------------------

bool AVXPlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (Params.Key.IsGamepadKey())
	{
		const bool bIsAxis = Params.Event == IE_Axis;
		if (false == bIsAxis || FMath::Abs(Params.AmountDepressed) >= GamepadSwitchThreshold)
		{
			SetInputDevice(EVoxelInputDevice::Gamepad);
		}
	}
	else if (Params.Event == IE_Pressed && (Params.Key.IsMouseButton() || false == Params.Key.IsAnalog()))
	{
		SetInputDevice(EVoxelInputDevice::KeyboardMouse);
	}

	return Super::InputKey(Params);
}

void AVXPlayerController::SetInputDevice(EVoxelInputDevice NewDevice)
{
	if (InputDevice == NewDevice)
	{
		return;
	}

	InputDevice = NewDevice;
	bShowMouseCursor = (NewDevice == EVoxelInputDevice::KeyboardMouse);
	OnInputDeviceChanged.Broadcast(NewDevice);
}

// ---------------------------------------------------------------------------
// 조준
// ---------------------------------------------------------------------------

FVector AVXPlayerController::ResolveAimDirection() const
{
	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == VoxelChar)
	{
		return FVector::ZeroVector;
	}

	if (InputDevice == EVoxelInputDevice::Gamepad)
	{
		// 오른쪽 스틱 > 이동 방향 > (없으면 마지막 조준 방향 유지)
		if (false == StickAimDirection.IsNearlyZero())
		{
			return StickAimDirection;
		}
		return VoxelChar->GetMoveInputDirection();
	}

	// 키보드·마우스: 커서가 가리키는 지면(플레이어 높이 평면) 지점을 향한다.
	FVector WorldOrigin, WorldDirection;
	if (false == DeprojectMousePositionToWorld(WorldOrigin, WorldDirection) || FMath::IsNearlyZero(WorldDirection.Z))
	{
		return FVector::ZeroVector;
	}

	const float PlaneZ = VoxelChar->GetActorLocation().Z;
	const float T = (PlaneZ - WorldOrigin.Z) / WorldDirection.Z;
	if (T < 0.f)
	{
		return FVector::ZeroVector;
	}

	const FVector Hit = WorldOrigin + WorldDirection * T;
	return (Hit - VoxelChar->GetActorLocation()).GetSafeNormal2D();
}

void AVXPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 마우스가 충분히 움직였으면 키보드·마우스로 전환
	float MouseX = 0.f, MouseY = 0.f;
	if (GetMousePosition(MouseX, MouseY))
	{
		const FVector2D Current(MouseX, MouseY);
		if (bHasLastMousePosition && FVector2D::Distance(Current, LastMousePosition) >= MouseSwitchThreshold)
		{
			SetInputDevice(EVoxelInputDevice::KeyboardMouse);
		}
		LastMousePosition = Current;
		bHasLastMousePosition = true;
	}

	if (bShowDebugInfo)
	{
		ShowDebugInfo();
	}

	AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == VoxelChar || VoxelChar->IsDead())
	{
		return;
	}

	const FVector Aim = ResolveAimDirection();
	if (false == Aim.IsNearlyZero())
	{
		VoxelChar->SetAimDirection(Aim);
	}

	if (UVoxelAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
	{
		ASC->ProcessHeldInputs();
	}
}

void AVXPlayerController::ShowDebugInfo() const
{
	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == GEngine || nullptr == VoxelChar)
	{
		return;
	}

	const float Health = VoxelChar->GetHealth();
	const float MaxHealth = VoxelChar->GetMaxHealth();
	const float Ratio = MaxHealth > 0.f ? Health / MaxHealth : 0.f;

	// 30% 이하는 빨강, 60% 이하는 주황 (저체력 경고)
	const FColor Color = Ratio <= 0.3f ? FColor::Red : (Ratio <= 0.6f ? FColor::Orange : FColor::Green);
	FString Text = VoxelChar->IsDead()
		? FString::Printf(TEXT("HP 0 / %.0f   [DEAD]"), MaxHealth)
		: FString::Printf(TEXT("HP %.0f / %.0f"), Health, MaxHealth);
	if (const UAbilitySystemComponent* ASC = VoxelChar->GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(VoxelTags::State_God))
		{
			Text += TEXT("   [GOD]");
		}
	}

	// 웨이브 표시(키 7001) 바로 아래에 오도록 다음 키를 쓴다.
	GEngine->AddOnScreenDebugMessage(7002, 0.f, Color, Text, true, FVector2D(1.5f, 1.5f));
}
