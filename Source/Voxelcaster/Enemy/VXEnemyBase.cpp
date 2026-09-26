// Copyright Epic Games, Inc. All Rights Reserved.

#include "Enemy/VXEnemyBase.h"
#include "Data/VXDataManager.h"
#include "AI/VXEnemyAIController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Data/VXEnemyData.h"
#include "Feel/VXGameFeelSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/VXHealthBarWidget.h"
#include "Voxelcaster.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AVXEnemyBase::AVXEnemyBase()
{
	// 피격 플래시·사망 연출 진행용
	PrimaryActorTick.bCanEverTick = true;

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

namespace
{
	TAutoConsoleVariable<int32> CVarDebugFlock(
		TEXT("VX.Debug.Flock"), 0,
		TEXT("1: 적 머리 위에 군집 힘을 그린다 (빨강 분리, 파랑 정렬, 초록 결합)"));
}

bool AVXEnemyBase::bEasyMode = false;
TArray<TWeakObjectPtr<AVXEnemyBase>> AVXEnemyBase::AliveEnemies;

float AVXEnemyBase::GetAdjustedAttackDamage(float BaseDamage) const
{
	return bEasyMode ? FMath::Max(BaseDamage - EasyModeDamageReduction, 0.f) : BaseDamage;
}

void AVXEnemyBase::PostInitializeComponents()
{
	// 부모(AVXCharacterBase)가 DefaultMaxHealth·DefaultMoveSpeed로 어트리뷰트를 초기화하므로 그 전에 적용한다.
	if (UVXDataManager* Data = UVXDataManager::Get())
	{
		if (const FVXEnemyRow* Row = Data->FindEnemy(StatRowName))
		{
			ApplyEnemyStats(*Row);
		}
	}

	Super::PostInitializeComponents();
}

void AVXEnemyBase::ApplyEnemyStats(const FVXEnemyRow& Row)
{
	DefaultMaxHealth = Row.MaxHealth;
	DefaultMoveSpeed = Row.MoveSpeed;

	// 군집: 음수(열 없음)면 코드 기본값 유지
	if (Row.FlockRadius >= 0.f)
	{
		FlockRadius = Row.FlockRadius;
	}
	if (Row.SeparationWeight >= 0.f)
	{
		SeparationWeight = Row.SeparationWeight;
	}
	if (Row.AlignmentWeight >= 0.f)
	{
		AlignmentWeight = Row.AlignmentWeight;
	}
	if (Row.CohesionWeight >= 0.f)
	{
		CohesionWeight = Row.CohesionWeight;
	}
}

void AVXEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	AliveEnemies.Add(this);

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
	VisualBaseScale = BodyScale;
	BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
	SetBodyColor(BodyColor);
}

bool AVXEnemyBase::HasSkeletalMesh() const
{
	return nullptr != GetMesh() && nullptr != GetMesh()->GetSkeletalMeshAsset();
}

void AVXEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AliveEnemies.RemoveAll([this](const TWeakObjectPtr<AVXEnemyBase>& Entry) { return false == Entry.IsValid() || Entry.Get() == this; });
	Super::EndPlay(EndPlayReason);
}

void AVXEnemyBase::ApplyFlocking()
{
	const bool bUseFlock = (AlignmentWeight > 0.f || CohesionWeight > 0.f) && FlockRadius > 0.f;
	if (SeparationWeight <= 0.f && false == bUseFlock)
	{
		return;
	}

	// 이웃 찾기: 살아 있는 적 목록 전체를 훑는다. (필드 최대 30마리라 가볍다)
	const FVector MyLocation = GetActorLocation();
	const float SeparationRadiusSq = SeparationRadius * SeparationRadius;
	const float FlockRadiusSq = FlockRadius * FlockRadius;

	FVector Separation = FVector::ZeroVector;
	FVector HeadingSum = FVector::ZeroVector;
	FVector CenterSum = FVector::ZeroVector;
	int32 FlockCount = 0;

	for (const TWeakObjectPtr<AVXEnemyBase>& Entry : AliveEnemies)
	{
		const AVXEnemyBase* Other = Entry.Get();
		if (nullptr == Other || Other == this || Other->IsDead() || Other->GetWorld() != GetWorld())
		{
			continue;
		}

		const FVector OtherLocation = Other->GetActorLocation();
		const FVector Away(MyLocation.X - OtherLocation.X, MyLocation.Y - OtherLocation.Y, 0.f);
		const float DistSq = Away.SizeSquared();

		// 분리: 종류와 상관없이 가까울수록 강하게. 완전히 겹치면 무작위 방향으로 떼어 낸다.
		if (DistSq < SeparationRadiusSq)
		{
			const FVector Direction = DistSq > KINDA_SMALL_NUMBER ? Away.GetSafeNormal()
				: FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
			Separation += Direction * (1.f - FMath::Sqrt(DistSq) / SeparationRadius);
		}

		// 정렬·결합: 같은 종류(DT_Enemies 행)끼리만 무리 짓는다.
		if (bUseFlock && DistSq < FlockRadiusSq && Other->StatRowName == StatRowName)
		{
			HeadingSum += Other->GetVelocity().GetSafeNormal2D();
			CenterSum += OtherLocation;
			++FlockCount;
		}
	}

	const FVector SeparationForce = Separation.GetClampedToMaxSize(1.f) * SeparationWeight;
	FVector AlignmentForce = FVector::ZeroVector;
	FVector CohesionForce = FVector::ZeroVector;

	if (FlockCount > 0)
	{
		// 플레이어 가까이에서는 무리를 풀어 둘러싸게 한다.
		float FlockScale = 1.f;
		if (const AVXCharacterBase* Player = FindLivePlayer())
		{
			const float DistToPlayer = FVector::Dist2D(MyLocation, Player->GetActorLocation());
			FlockScale = FMath::GetMappedRangeValueClamped(FVector2D(FlockReleaseDistance, FlockReleaseDistance * 2.f), FVector2D(0.f, 1.f), DistToPlayer);
		}

		AlignmentForce = HeadingSum.GetSafeNormal2D() * AlignmentWeight * FlockScale;

		// 결합: 무리 중심까지 멀수록 강하게 (반경 끝에서 최대)
		const FVector ToCenter = CenterSum / FlockCount - MyLocation;
		const float CenterPull = FMath::Min(ToCenter.Size2D() / FlockRadius, 1.f);
		CohesionForce = ToCenter.GetSafeNormal2D() * CenterPull * CohesionWeight * FlockScale;
	}

	// 추적 입력(크기 1)과 합쳐진 뒤 이동 컴포넌트가 크기 1로 자른다 → 방향이 섞인다.
	const FVector Steer = SeparationForce + AlignmentForce + CohesionForce;
	if (false == Steer.IsNearlyZero())
	{
		AddMovementInput(Steer, 1.f);
	}

#if ENABLE_DRAW_DEBUG
	if (CVarDebugFlock.GetValueOnGameThread() > 0)
	{
		const FVector Origin = MyLocation + FVector(0.f, 0.f, 120.f);
		const float Length = 150.f;
		DrawDebugLine(GetWorld(), Origin, Origin + SeparationForce * Length, FColor::Red, false, 0.f, 0, 3.f);
		DrawDebugLine(GetWorld(), Origin, Origin + AlignmentForce * Length, FColor::Blue, false, 0.f, 0, 3.f);
		DrawDebugLine(GetWorld(), Origin, Origin + CohesionForce * Length, FColor::Green, false, 0.f, 0, 3.f);
	}
#endif
}

USceneComponent* AVXEnemyBase::GetVisualMesh() const
{
	return HasSkeletalMesh() ? static_cast<USceneComponent*>(GetMesh()) : static_cast<USceneComponent*>(BodyMesh);
}

void AVXEnemyBase::HandleDamaged(float Amount)
{
	// 피격 플래시 0.1초: 흰색(임시 큐브) + 크기 펀치(모든 메시). 끝나는 시점은 실제 시간 (히트스톱 중에도 0.1초)
	const bool bAlreadyFlashing = FlashEndRealTime > 0.0;
	FlashEndRealTime = GetWorld()->GetRealTimeSeconds() + 0.1;

	if (USceneComponent* Visual = GetVisualMesh())
	{
		if (false == bAlreadyFlashing)
		{
			VisualBaseScale = Visual->GetRelativeScale3D();
		}
		Visual->SetRelativeScale3D(VisualBaseScale * 1.15f);
	}
	SetBodyColor(FLinearColor::White);
}

void AVXEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (false == IsDead())
	{
		ApplyFlocking();
	}

	USceneComponent* Visual = GetVisualMesh();

	// 피격 플래시 종료
	if (FlashEndRealTime > 0.0 && GetWorld()->GetRealTimeSeconds() >= FlashEndRealTime)
	{
		FlashEndRealTime = 0.0;
		if (Visual)
		{
			Visual->SetRelativeScale3D(VisualBaseScale);
		}
		SetBodyColor(BodyColor);
	}

	// 사망 연출: 사망 애니메이션이 없으면 수명 동안 작아지며 사라진다
	if (DeathWorldTime >= 0.f && Visual && (nullptr == DeathMontage || false == HasSkeletalMesh()))
	{
		const float Alpha = FMath::Clamp((GetWorld()->GetTimeSeconds() - DeathWorldTime) / FMath::Max(DeathLifeSpan, 0.01f), 0.f, 1.f);
		Visual->SetRelativeScale3D(VisualBaseScale * FMath::Max(1.f - Alpha, 0.05f));
	}
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

	// 처치 타격감: 히트스톱, 엘리트는 진동 (DES-FEEL-001)
	FlashEndRealTime = 0.0;
	DeathWorldTime = GetWorld()->GetTimeSeconds();
	AliveEnemies.RemoveAll([this](const TWeakObjectPtr<AVXEnemyBase>& Entry) { return false == Entry.IsValid() || Entry.Get() == this; });
	if (UVXGameFeelSubsystem* Feel = UVXGameFeelSubsystem::Get(this))
	{
		Feel->RequestHitStop(KillHitStop);
		if (KillVibrationIntensity > 0.f)
		{
			Feel->PlayVibration(UGameplayStatics::GetPlayerController(this, 0), KillVibrationIntensity, KillVibrationDuration);
		}
	}

	float LifeSpan = DeathLifeSpan;
	if (DeathMontage && HasSkeletalMesh())
	{
		const float MontageLength = PlayAnimMontage(DeathMontage);
		LifeSpan = FMath::Max(LifeSpan, MontageLength);
	}
	SetLifeSpan(LifeSpan);
}
