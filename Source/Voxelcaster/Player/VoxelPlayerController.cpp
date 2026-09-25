// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/VoxelPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Character/VoxelCharacterBase.h"
#include "Core/VoxelGameMode.h"
#include "Enemy/VoxelElite.h"
#include "Enemy/VoxelRunner.h"
#include "Enemy/VoxelShooter.h"
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
#include "Wave/VoxelWaveManager.h"
#include "Voxelcaster.h"

AVXPlayerController::AVXPlayerController()
{
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

// ---------------------------------------------------------------------------
// 디버그 콘솔 명령
// ---------------------------------------------------------------------------

void AVXPlayerController::DebugDamage(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 10.f;
	}
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		VoxelEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), Amount);
	}
}

void AVXPlayerController::DebugHeal(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 30.f;
	}
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		VoxelEffects::ApplyHeal(VoxelChar->GetAbilitySystemComponent(), Amount);
	}
}

void AVXPlayerController::DebugKill()
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		VoxelEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), VoxelChar->GetMaxHealth() * 10.f);
	}
}

void AVXPlayerController::DebugSpawnRunners(int32 Count)
{
	// Exec 명령은 C++ 기본 인자를 적용하지 않는다. 인자를 생략하면 0이 들어오므로 기본값으로 보정한다.
	if (Count <= 0)
	{
		Count = 10;
	}

	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == VoxelChar || nullptr == GetWorld())
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Distance = FMath::FRandRange(800.f, 1200.f);
		const FVector Offset = FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * Distance;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AVXRunner>(AVXRunner::StaticClass(), VoxelChar->GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
	}
}

void AVXPlayerController::DebugSpawnEnemy(const FString& EnemyType, int32 Count)
{
	if (Count <= 0)
	{
		Count = 1;
	}

	TSubclassOf<AVXEnemyBase> EnemyClass;
	if (EnemyType.Equals(TEXT("Runner"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXRunner::StaticClass();
	}
	else if (EnemyType.Equals(TEXT("Shooter"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXShooter::StaticClass();
	}
	else if (EnemyType.Equals(TEXT("Elite"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXElite::StaticClass();
	}

	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == EnemyClass.Get() || nullptr == VoxelChar)
	{
		UE_LOG(LogVoxel, Warning, TEXT("DebugSpawnEnemy: unknown type '%s' (Runner, Shooter, Elite)"), *EnemyType);
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Distance = FMath::FRandRange(800.f, 1200.f);
		const FVector Offset = FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * Distance;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AVXEnemyBase>(EnemyClass, VoxelChar->GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
	}
}

void AVXPlayerController::DebugDamageEnemies(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 20.f;
	}

	for (TActorIterator<AVXCharacterBase> It(GetWorld()); It; ++It)
	{
		if (It->GetTeam() == EVXTeam::Enemy && false == It->IsDead())
		{
			VoxelEffects::ApplyDamage(It->GetAbilitySystemComponent(), Amount);
		}
	}
}

void AVXPlayerController::DebugKillEnemies()
{
	for (TActorIterator<AVXCharacterBase> It(GetWorld()); It; ++It)
	{
		if (It->GetTeam() == EVXTeam::Enemy && false == It->IsDead())
		{
			VoxelEffects::ApplyDamage(It->GetAbilitySystemComponent(), It->GetMaxHealth() * 10.f);
		}
	}
}

void AVXPlayerController::DebugStartWave(int32 WaveIndex)
{
	if (WaveIndex <= 0)
	{
		WaveIndex = 1;
	}

	if (const AVoxelGameMode* GameMode = GetWorld()->GetAuthGameMode<AVoxelGameMode>())
	{
		GameMode->GetWaveManager()->StartWave(WaveIndex);
	}
}

void AVXPlayerController::DebugStopWaves()
{
	if (const AVoxelGameMode* GameMode = GetWorld()->GetAuthGameMode<AVoxelGameMode>())
	{
		GameMode->GetWaveManager()->StopWaves();
	}
}
