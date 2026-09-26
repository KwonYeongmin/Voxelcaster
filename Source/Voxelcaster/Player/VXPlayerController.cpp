// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/VXPlayerController.h"
#include "AbilitySystemComponent.h"
#include "Character/VXCharacterBase.h"
#include "Core/VXGameMode.h"
#include "Enemy/VXElite.h"
#include "Enemy/VXEnemyBase.h"
#include "Enemy/VXRunner.h"
#include "Enemy/VXShooter.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "GAS/VXAttributeSet.h"
#include "GAS/VXGameplayEffects.h"
#include "GAS/VXGameplayTags.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Cheat/VXCheatManager.h"
#include "Modifier/VXModifierComponent.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "UI/VXHUDWidget.h"
#include "UI/VXText.h"
#include "UI/ViewModel/VX_VM_Hud.h"
#include "GameplayEffect.h"
#include "UI/VXPauseWidget.h"
#include "UI/VXResultWidget.h"
#include "UI/VXRewardSelectWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Wave/VXWaveManager.h"
#include "Voxelcaster.h"

AVXPlayerController::AVXPlayerController()
{
	CheatClass = UVXCheatManager::StaticClass();

	HUDWidgetClass = TSoftClassPtr<UVXHUDWidget>(FSoftObjectPath(TEXT("/Game/Voxelcaster/UI/WBP_VX_HUD.WBP_VX_HUD_C")));
	RewardSelectWidgetClass = TSoftClassPtr<UVXRewardSelectWidget>(FSoftObjectPath(TEXT("/Game/Voxelcaster/UI/WBP_VX_RewardSelect.WBP_VX_RewardSelect_C")));
	PauseWidgetClass = TSoftClassPtr<UVXPauseWidget>(FSoftObjectPath(TEXT("/Game/Voxelcaster/UI/WBP_VX_Pause.WBP_VX_Pause_C")));
	ResultWidgetClass = TSoftClassPtr<UVXResultWidget>(FSoftObjectPath(TEXT("/Game/Voxelcaster/UI/WBP_VX_Result.WBP_VX_Result_C")));

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

namespace
{
	/** WBP 클래스가 있으면 그것을, 없으면 C++ 기본 화면 클래스를 쓴다 */
	template<typename T>
	TSubclassOf<T> ResolveWidgetClass(const TSoftClassPtr<T>& SoftClass)
	{
		if (false == SoftClass.IsNull())
		{
			if (UClass* Loaded = SoftClass.LoadSynchronous())
			{
				return Loaded;
			}
		}
		return T::StaticClass();
	}
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
	PauseAction = MakeAction(TEXT("IA_Pause"), EInputActionValueType::Boolean);

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
	// 일시정지: Esc / 패드 Menu(Options)
	GameplayContext->MapKey(PauseAction, EKeys::Escape);
	GameplayContext->MapKey(PauseAction, EKeys::Gamepad_Special_Right);
}

void AVXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UVXUpgradeSubsystem* Upgrades = GetWorld()->GetSubsystem<UVXUpgradeSubsystem>())
	{
		Upgrades->OnChoicesReady.AddUObject(this, &AVXPlayerController::HandleChoicesReady);
	}

	if (AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
	{
		GameMode->GetWaveManager()->OnGameWon.AddUObject(this, &AVXPlayerController::HandleGameEnded, true);
		GameMode->GetWaveManager()->OnGameLost.AddUObject(this, &AVXPlayerController::HandleGameEnded, false);
	}

	// 전투 HUD (CommonUI)
	if (IsLocalController())
	{
		HudViewModel = NewObject<UVX_VM_Hud>(this);
		HUDWidget = CreateWidget<UVXHUDWidget>(this, ResolveWidgetClass(HUDWidgetClass));
		if (HUDWidget)
		{
			HUDWidget->SetViewModel(HudViewModel);
			HUDWidget->AddToViewport(0);
		}
	}
}

// ---------------------------------------------------------------------------
// 메뉴 (일시정지·결과)
// ---------------------------------------------------------------------------

void AVXPlayerController::HandlePause(const FInputActionValue& Value)
{
	// 메뉴가 열려 있을 때는 메뉴가 입력을 받으므로 여기로 오지 않는다. (UI 전용 입력 모드)
	const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>();
	if (IsMenuOpen() || (GameMode && EVXWaveState::Finished == GameMode->GetWaveManager()->GetState()))
	{
		return;
	}

	OpenMenu(CreateWidget<UVXPauseWidget>(this, ResolveWidgetClass(PauseWidgetClass)));
}

void AVXPlayerController::OpenMenu(UCommonActivatableWidget* Menu)
{
	if (nullptr == Menu)
	{
		return;
	}

	CloseMenu();
	CurrentMenu = Menu;
	Menu->AddToViewport(50);
	Menu->ActivateWidget();
	SetPause(true);

	SetInputMode(FInputModeUIOnly());
	if (UWidget* Focus = Menu->GetDesiredFocusTarget())
	{
		Focus->SetFocus();
	}
}

void AVXPlayerController::CloseMenu()
{
	if (nullptr == CurrentMenu)
	{
		return;
	}

	CurrentMenu->DeactivateWidget();
	CurrentMenu->RemoveFromParent();
	CurrentMenu = nullptr;

	SetPause(false);
	FInputModeGameOnly InputMode;
	InputMode.SetConsumeCaptureMouseDown(false);
	SetInputMode(InputMode);
}

void AVXPlayerController::RestartGame()
{
	SetPause(false);
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void AVXPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void AVXPlayerController::HandleGameEnded(bool bVictory)
{
	// 사망 연출이 보이도록 잠시 뒤에 결과 화면을 연다.
	FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AVXPlayerController::ShowResult, bVictory);
	GetWorldTimerManager().SetTimer(ResultTimer, Delegate, bVictory ? 0.5f : 1.f, false);
}

void AVXPlayerController::ShowResult(bool bVictory)
{
	UVXResultWidget* Result = CreateWidget<UVXResultWidget>(this, ResolveWidgetClass(ResultWidgetClass));
	if (nullptr == Result)
	{
		return;
	}

	FVXRunResult Run;
	Run.bVictory = bVictory;
	if (const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
	{
		const UVXWaveManager* Waves = GameMode->GetWaveManager();
		Run.ReachedWave = Waves->GetCurrentWave();
		Run.TotalWaves = Waves->GetTotalWaves();
		Run.Kills = Waves->GetKillCount();
		Run.PlayTime = Waves->GetPlayTime();
	}

	const UVXModifierComponent* Modifiers = nullptr != GetPawn() ? GetPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr;
	Result->SetResult(Run, Modifiers);
	OpenMenu(Result);
}

void AVXPlayerController::HandleChoicesReady(const TArray<FVXUpgradeCard>& Choices)
{
	OpenRewardSelect(Choices);
}

void AVXPlayerController::OpenRewardSelect(const TArray<FVXUpgradeCard>& Choices)
{
	UVXRewardSelectWidget* Widget = CreateWidget<UVXRewardSelectWidget>(this, ResolveWidgetClass(RewardSelectWidgetClass));
	if (nullptr == Widget)
	{
		return;
	}

	int32 WaveIndex = 0;
	if (const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
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

	BindAbility(Skill1Action, VXTags::Input_Skill1);
	BindAbility(Skill2Action, VXTags::Input_Skill2);
	BindAbility(Skill3Action, VXTags::Input_Skill3);
	BindAbility(DashAction, VXTags::Input_Dash);

	EnhancedInput->BindAction(PauseAction, ETriggerEvent::Started, this, &AVXPlayerController::HandlePause);
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
		if (UVXAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
		{
			ASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void AVXPlayerController::HandleAbilityReleased(FGameplayTag InputTag)
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPawn()))
	{
		if (UVXAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
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
			SetInputDevice(EVXInputDevice::Gamepad);
		}
	}
	else if (Params.Event == IE_Pressed && (Params.Key.IsMouseButton() || false == Params.Key.IsAnalog()))
	{
		SetInputDevice(EVXInputDevice::KeyboardMouse);
	}

	return Super::InputKey(Params);
}

void AVXPlayerController::SetInputDevice(EVXInputDevice NewDevice)
{
	if (InputDevice == NewDevice)
	{
		return;
	}

	InputDevice = NewDevice;
	bShowMouseCursor = (NewDevice == EVXInputDevice::KeyboardMouse);
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

	if (InputDevice == EVXInputDevice::Gamepad)
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
			SetInputDevice(EVXInputDevice::KeyboardMouse);
		}
		LastMousePosition = Current;
		bHasLastMousePosition = true;
	}

	if (bShowDebugInfo)
	{
		ShowDebugInfo();
	}

	UpdateHudViewModel();

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

	if (UVXAbilitySystemComponent* ASC = VoxelChar->GetVoxelAbilitySystemComponent())
	{
		ASC->ProcessHeldInputs();
	}
}

void AVXPlayerController::UpdateHudViewModel()
{
	const AVXCharacterBase* VoxelPlayer = Cast<AVXCharacterBase>(GetPawn());
	if (nullptr == HudViewModel || nullptr == VoxelPlayer)
	{
		return;
	}

	// ---- 체력
	const float Health = VoxelPlayer->GetHealth();
	const float MaxHealth = VoxelPlayer->GetMaxHealth();
	const float Ratio = MaxHealth > 0.f ? Health / MaxHealth : 0.f;
	const UAbilitySystemComponent* ASC = VoxelPlayer->GetAbilitySystemComponent();

	FString HealthString = FString::Printf(TEXT("HP %.0f / %.0f"), Health, MaxHealth);
	if (ASC && ASC->HasMatchingGameplayTag(VXTags::State_God))
	{
		HealthString += TEXT("   [GOD]");
	}
	if (AVXEnemyBase::IsEasyMode())
	{
		HealthString += TEXT("   [EASY]");
	}
	HudViewModel->SetHealthPercent(Ratio);
	HudViewModel->SetbLowHealth(Ratio <= 0.3f);
	HudViewModel->SetHealthText(FText::FromString(HealthString));

	// ---- 웨이브
	if (const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
	{
		const UVXWaveManager* Waves = GameMode->GetWaveManager();
		HudViewModel->SetWaveText(FText::FromString(VXText::Format(TEXT("UI.Wave"), { Waves->GetCurrentWave(), Waves->GetTotalWaves() })));
		HudViewModel->SetEnemiesText(FText::FromString(
			VXText::Format(TEXT("UI.Remaining"), { Waves->GetRemainingEnemies() }) + TEXT("     ") +
			VXText::Format(TEXT("UI.Kills"), { Waves->GetKillCount() })));
	}

	// ---- 스킬 슬롯 (스킬 3 + 대시)
	if (nullptr == ASC)
	{
		return;
	}

	struct FSlotDef { UVX_VM_SkillSlot* ViewModel; FGameplayTag Tag; const TCHAR* NameKey; const TCHAR* Kbm; const TCHAR* Pad; };
	const FSlotDef Defs[] =
	{
		{ HudViewModel->GetSkillSlot0(), VXTags::Cooldown_MagicBolt,  TEXT("Skill.MagicBolt"),  TEXT("LMB"),   TEXT("RT") },
		{ HudViewModel->GetSkillSlot1(), VXTags::Cooldown_Nova,       TEXT("Skill.Nova"),       TEXT("RMB"),   TEXT("LT") },
		{ HudViewModel->GetSkillSlot2(), VXTags::Cooldown_BladeSweep, TEXT("Skill.BladeSweep"), TEXT("Q"),     TEXT("RB") },
		{ HudViewModel->GetSkillSlot3(), VXTags::Cooldown_Dash,       TEXT("UI.Dash"),          TEXT("Space"), TEXT("A") },
	};

	const bool bGamepad = EVXInputDevice::Gamepad == InputDevice;
	const UVXModifierComponent* Modifiers = VoxelPlayer->FindComponentByClass<UVXModifierComponent>();

	for (const FSlotDef& Def : Defs)
	{
		UVX_VM_SkillSlot* Slot = Def.ViewModel;
		if (nullptr == Slot)
		{
			continue;
		}

		// 입력 키는 마지막으로 쓴 장치에 맞춘다 (DES-CTRL-001)
		Slot->SetKeyText(FText::FromString(bGamepad ? Def.Pad : Def.Kbm));
		Slot->SetSkillName(FText::FromString(VXText::Get(Def.NameKey)));

		// 쿨다운: 이 태그를 부여하는 활성 GE의 남은 시간
		float Remaining = 0.f;
		float Duration = 0.f;
		const FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Def.Tag));
		for (const TPair<float, float>& Pair : ASC->GetActiveEffectsTimeRemainingAndDuration(Query))
		{
			if (Pair.Key > Remaining)
			{
				Remaining = Pair.Key;
				Duration = Pair.Value;
			}
		}

		const bool bReady = Remaining <= 0.f;
		Slot->SetbReady(bReady);
		Slot->SetCooldownPercent(bReady || Duration <= 0.f ? 1.f : 1.f - Remaining / Duration);
		// 0.1초 단위로 바뀌므로 알림도 그때만 간다
		Slot->SetCooldownText(bReady ? FText::GetEmpty() : FText::FromString(FString::Printf(TEXT("%.1f"), Remaining)));

		FString ModText;
		if (const FVXModifierSlots* Slots = nullptr != Modifiers ? Modifiers->FindSlots(Def.Tag) : nullptr)
		{
			for (const EVXModifierType Type : Slots->Slots)
			{
				ModText += FString::Printf(TEXT("[%s]"), *UVXModifierComponent::GetModifierDisplayName(Type));
			}
		}
		Slot->SetModifiersText(FText::FromString(ModText));
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
		if (ASC->HasMatchingGameplayTag(VXTags::State_God))
		{
			Text += TEXT("   [GOD]");
		}
	}

	// 웨이브 표시(키 7001) 바로 아래에 오도록 다음 키를 쓴다.
	GEngine->AddOnScreenDebugMessage(7002, 0.f, Color, Text, true, FVector2D(1.5f, 1.5f));
}
