// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VoxelPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/Abilities/VoxelGA_BladeSweep.h"
#include "GAS/Abilities/VoxelGA_Dash.h"
#include "GAS/Abilities/VoxelGA_MagicBolt.h"
#include "GAS/Abilities/VoxelGA_Nova.h"
#include "GAS/VoxelAbilitySystemComponent.h"
#include "GAS/VoxelGameplayTags.h"
#include "UObject/ConstructorHelpers.h"

AVXPlayerCharacter::AVXPlayerCharacter()
{
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

void AVXPlayerCharacter::GrantStartupAbilities()
{
	UVoxelAbilitySystemComponent* ASC = GetVoxelAbilitySystemComponent();
	ASC->GiveAbilityWithInput(UVoxelGA_Dash::StaticClass(), VoxelTags::Input_Dash);

	// 스킬 3종: 스킬 1 = 매직 볼트(좌클릭/RT), 스킬 2 = 노바(우클릭/LT), 스킬 3 = 블레이드 스윕(Q/RB)
	ASC->GiveAbilityWithInput(UVoxelGA_MagicBolt::StaticClass(), VoxelTags::Input_Skill1);
	ASC->GiveAbilityWithInput(UVoxelGA_Nova::StaticClass(), VoxelTags::Input_Skill2);
	ASC->GiveAbilityWithInput(UVoxelGA_BladeSweep::StaticClass(), VoxelTags::Input_Skill3);
}
