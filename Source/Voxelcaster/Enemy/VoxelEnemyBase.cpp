// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VoxelEnemyBase.h"
#include "AI/VXEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/VXHealthBarWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AVXEnemyBase::AVXEnemyBase()
{
	Team = EVXTeam::Enemy;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AVXEnemyAIController::StaticClass();

	// 머리 위 HP 바. 화면 공간이라 카메라 각도와 상관없이 항상 정면으로 보인다.
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetWidgetClass(UVXHealthBarWidget::StaticClass());
	HealthBarComponent->SetDrawSize(FVector2D(70.f, 7.f));
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	// HP 바는 캡슐 위쪽에 띄운다 (적마다 키가 달라서 캡슐 높이 기준)
	HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 30.f));
	HealthBarComponent->InitWidget();
	OnHealthChanged.AddUObject(this, &AVXEnemyBase::UpdateHealthBar);
	UpdateHealthBar(GetHealth(), GetMaxHealth());

	// 실제 캐릭터 메시가 있으면 임시 큐브를 숨긴다.
	if (HasSkeletalMesh())
	{
		BodyMesh->SetVisibility(false);
		BodyMesh->SetHiddenInGame(true);
		return;
	}

	BodyMesh->SetRelativeScale3D(BodyScale);
	BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
	SetBodyColor(BodyColor);
}

bool AVXEnemyBase::HasSkeletalMesh() const
{
	return nullptr != GetMesh() && nullptr != GetMesh()->GetSkeletalMeshAsset();
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

void AVXEnemyBase::UpdateHealthBar(float Current, float Max)
{
	if (UVXHealthBarWidget* Bar = Cast<UVXHealthBarWidget>(HealthBarComponent->GetUserWidgetObject()))
	{
		Bar->SetHealth(Current, Max);
	}
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
	HealthBarComponent->SetVisibility(false);

	float LifeSpan = DeathLifeSpan;
	if (DeathMontage && HasSkeletalMesh())
	{
		const float MontageLength = PlayAnimMontage(DeathMontage);
		LifeSpan = FMath::Max(LifeSpan, MontageLength);
	}
	SetLifeSpan(LifeSpan);
}
