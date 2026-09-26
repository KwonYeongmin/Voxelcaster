// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VXPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/Abilities/VX_GA_BladeSweep.h"
#include "GAS/Abilities/VX_GA_Dash.h"
#include "GAS/Abilities/VX_GA_MagicBolt.h"
#include "GAS/Abilities/VX_GA_Nova.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "GAS/VXGameplayTags.h"
#include "Modifier/VXModifierComponent.h"
#include "Feel/VXGameFeelSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

AVXPlayerCharacter::AVXPlayerCharacter()
{
	// 화면 흔들림 진행용
	PrimaryActorTick.bCanEverTick = true;

	// 탑다운 카메라: 피치 -55도, 거리 14m, 회전 고정 (플레이어를 따라 이동만 한다)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->bUsePawnControlRotation = false;
	TopDownCamera->FieldOfView = 50.f;

	ModifierComponent = CreateDefaultSubobject<UVXModifierComponent>(TEXT("ModifierComponent"));

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.6f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AVXPlayerCharacter::HandleDamaged(float Amount)
{
	// 피격 타격감: 히트스톱, 진동(패드), 화면 흔들림 (DES-FEEL-001). 대시 무적 중에는 피해 자체가 없어 호출되지 않는다.
	if (UVXGameFeelSubsystem* Feel = UVXGameFeelSubsystem::Get(this))
	{
		Feel->RequestHitStop(HitStopOnDamaged);
		Feel->PlayVibration(Cast<APlayerController>(GetController()), DamagedVibrationIntensity, DamagedVibrationDuration);
	}

	if (ShakeEndRealTime <= 0.0)
	{
		BaseSocketOffset = CameraBoom->SocketOffset;
	}
	ShakeEndRealTime = GetWorld()->GetRealTimeSeconds() + ShakeDuration;
}

void AVXPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ShakeEndRealTime <= 0.0)
	{
		return;
	}

	// 실제 시간 기준으로 줄어드는 무작위 카메라 흔들림 (히트스톱 중에도 흔들린다)
	const double Remaining = ShakeEndRealTime - GetWorld()->GetRealTimeSeconds();
	if (Remaining <= 0.0)
	{
		ShakeEndRealTime = 0.0;
		CameraBoom->SocketOffset = BaseSocketOffset;
		return;
	}

	const float Strength = ShakeAmplitude * static_cast<float>(Remaining / FMath::Max(ShakeDuration, 0.01f));
	CameraBoom->SocketOffset = BaseSocketOffset + FVector(0.f, FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f)) * Strength;
}

void AVXPlayerCharacter::GrantStartupAbilities()
{
	UVXAbilitySystemComponent* ASC = GetVoxelAbilitySystemComponent();
	ASC->GiveAbilityWithInput(UVX_GA_Dash::StaticClass(), VXTags::Input_Dash);

	// 스킬 3종: 스킬 1 = 매직 볼트(좌클릭/RT), 스킬 2 = 노바(우클릭/LT), 스킬 3 = 블레이드 스윕(Q/RB)
	ASC->GiveAbilityWithInput(UVX_GA_MagicBolt::StaticClass(), VXTags::Input_Skill1);
	ASC->GiveAbilityWithInput(UVX_GA_Nova::StaticClass(), VXTags::Input_Skill2);
	ASC->GiveAbilityWithInput(UVX_GA_BladeSweep::StaticClass(), VXTags::Input_Skill3);
}
