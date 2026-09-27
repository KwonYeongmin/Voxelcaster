// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VXPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Character/VXCameraOcclusionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
	CameraOcclusion = CreateDefaultSubobject<UVXCameraOcclusionComponent>(TEXT("CameraOcclusion"));

	// 외형은 BP_VXCharacter의 Mesh(스켈레탈 메시)로 지정한다
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

	TickAttackBounce(DeltaSeconds);

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

void AVXPlayerCharacter::PlayAttackBounce(float Strength)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (nullptr == MeshComponent || AttackBounceAmount <= 0.f)
	{
		return;
	}

	// 처음 한 번 BP에서 맞춘 위치·크기를 기억한다 (출렁이는 중에 다시 눌러도 원래 값 기준)
	if (false == bMeshBaseCaptured)
	{
		MeshBaseLocation = MeshComponent->GetRelativeLocation();
		MeshBaseScale = MeshComponent->GetRelativeScale3D();
		bMeshBaseCaptured = true;
	}

	BounceElapsed = 0.f;
	BounceStrength = Strength;
}

void AVXPlayerCharacter::TickAttackBounce(float DeltaSeconds)
{
	if (BounceElapsed < 0.f)
	{
		return;
	}

	// 게임 시간 기준: 히트스톱 중에는 같이 멈춘다
	BounceElapsed += DeltaSeconds;
	if (BounceElapsed >= AttackBounceDuration)
	{
		BounceElapsed = -1.f;
		ApplyMeshSquash(1.f, 1.f);
		return;
	}

	// 감쇠 진동: 처음에 위로 늘어나고(+) → 눌리고(-) → 점점 작아진다
	const float Wave = FMath::Cos(2.f * PI * AttackBounceFrequency * BounceElapsed);
	const float Stretch = AttackBounceAmount * BounceStrength * FMath::Exp(-AttackBounceDamping * BounceElapsed) * Wave;
	const float ScaleZ = FMath::Max(1.f + Stretch, 0.3f);
	const float ScaleXY = 1.f / FMath::Sqrt(ScaleZ);
	ApplyMeshSquash(ScaleXY, ScaleZ);
}

void AVXPlayerCharacter::ApplyMeshSquash(float ScaleXY, float ScaleZ)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (nullptr == MeshComponent || false == bMeshBaseCaptured)
	{
		return;
	}

	// 캡슐 바닥 중앙(발밑)을 고정점으로 늘리고 줄인다. 메시 원점이 캐릭터 밖에 있어도 옆으로 밀리지 않는다.
	// (메시 회전이 Yaw만 있을 때 정확하다. XY를 같은 비율로 바꾸므로 Yaw와 무관)
	const FVector Pivot(0.f, 0.f, -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	const FVector Offset = MeshBaseLocation - Pivot;
	MeshComponent->SetRelativeLocation(Pivot + FVector(Offset.X * ScaleXY, Offset.Y * ScaleXY, Offset.Z * ScaleZ));
	MeshComponent->SetRelativeScale3D(MeshBaseScale * FVector(ScaleXY, ScaleXY, ScaleZ));
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
