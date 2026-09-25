// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VoxelEnemyBase.h"
#include "AI/VXEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AVXEnemyBase::AVXEnemyBase()
{
	Team = EVXTeam::Enemy;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AVXEnemyAIController::StaticClass();

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void AVXEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	BodyMesh->SetRelativeScale3D(BodyScale);
	BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
	SetBodyColor(BodyColor);
}

void AVXEnemyBase::SetBodyColor(const FLinearColor& Color)
{
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}
}

bool AVXEnemyBase::IsDrivenByStateTree() const
{
	const AVXEnemyAIController* AIController = Cast<AVXEnemyAIController>(GetController());
	return nullptr != AIController && AIController->IsStateTreeRunning();
}

AVXCharacterBase* AVXEnemyBase::FindLivePlayer() const
{
	AVXCharacterBase* Player = Cast<AVXCharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	return (Player && false == Player->IsDead()) ? Player : nullptr;
}

void AVXEnemyBase::HandleDeath()
{
	Super::HandleDeath();

	// 죽은 적이 스킬·이동을 막지 않도록 충돌을 끄고 곧 제거한다. (연출은 game-feel spec에서)
	SetActorEnableCollision(false);
	SetLifeSpan(DeathLifeSpan);
}
