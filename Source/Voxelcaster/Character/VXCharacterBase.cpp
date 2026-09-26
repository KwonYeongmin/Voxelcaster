// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/VXCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "GAS/VXAttributeSet.h"
#include "GAS/VXGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Voxelcaster.h"

namespace
{
	/** 1이면 체력이 바뀔 때 캐릭터 머리 위에 HP를 표시하고 로그로 남긴다 */
	TAutoConsoleVariable<int32> CVarShowHealth(
		TEXT("Voxel.Debug.ShowHealth"), 0,
		TEXT("1: 체력이 바뀔 때 머리 위에 HP를 표시하고 로그로 남긴다"));
}

AVXCharacterBase::AVXCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UVXAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UVXAttributeSet>(TEXT("AttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet.Get());

	// 탑다운: 조준 방향을 컨트롤러가 직접 지정하므로 컨트롤러 회전을 따르지 않는다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->bConstrainToPlane = true;
	Movement->SetPlaneConstraintNormal(FVector::UpVector);
	Movement->MaxAcceleration = 4000.f;
	Movement->BrakingDecelerationWalking = 4000.f;
	Movement->GroundFriction = 12.f;

	// 캡슐 반경 0.4m (design: DES-CHAR-001)
	GetCapsuleComponent()->InitCapsuleSize(40.f, 90.f);
}

UAbilitySystemComponent* AVXCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float AVXCharacterBase::GetHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

float AVXCharacterBase::GetMaxHealth() const
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

void AVXCharacterBase::SetAimDirection(const FVector& NewAimDirection)
{
	const FVector Flat = NewAimDirection.GetSafeNormal2D();
	if (Flat.IsNearlyZero())
	{
		return;
	}

	AimDirection = Flat;
	if (false == bIsDead)
	{
		SetActorRotation(Flat.Rotation());
	}
}

FVector AVXCharacterBase::GetDashDirection() const
{
	return MoveInputDirection.IsNearlyZero() ? AimDirection : MoveInputDirection.GetSafeNormal2D();
}

void AVXCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	AttributeSet->InitMaxHealth(DefaultMaxHealth);
	AttributeSet->InitHealth(DefaultMaxHealth);
	AttributeSet->InitMoveSpeed(DefaultMoveSpeed);
	GetCharacterMovement()->MaxWalkSpeed = DefaultMoveSpeed;
}

void AVXCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UVXAttributeSet::GetHealthAttribute())
		.AddUObject(this, &AVXCharacterBase::OnHealthAttributeChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UVXAttributeSet::GetMoveSpeedAttribute())
		.AddUObject(this, &AVXCharacterBase::OnMoveSpeedAttributeChanged);

	AbilitySystemComponent->AbilityFailedCallbacks.AddLambda([](const UGameplayAbility* Ability, const FGameplayTagContainer& FailureTags)
	{
		UE_LOG(LogVX, Warning, TEXT("Ability failed: %s tags=[%s]"), *GetNameSafe(Ability), *FailureTags.ToStringSimple());
	});

	GrantStartupAbilities();
	UE_LOG(LogVX, Log, TEXT("%s granted %d abilities"), *GetName(), AbilitySystemComponent->GetActivatableAbilities().Num());

	OnHealthChanged.Broadcast(GetHealth(), GetMaxHealth());
}

void AVXCharacterBase::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue, GetMaxHealth());

	if (CVarShowHealth.GetValueOnGameThread() > 0)
	{
		const FString Text = FString::Printf(TEXT("%.0f / %.0f"), Data.NewValue, GetMaxHealth());
		UE_LOG(LogVX, Log, TEXT("%s HP %s (%+.0f)"), *GetName(), *Text, Data.NewValue - Data.OldValue);
#if ENABLE_DRAW_DEBUG
		const FColor Color = Team == EVXTeam::Player ? FColor::Green : FColor::Red;
		DrawDebugString(GetWorld(), FVector(0.f, 0.f, 150.f), Text, this, Color, 1.f, true, 1.5f);
#endif
	}

	if (Data.NewValue <= 0.f && false == bIsDead)
	{
		HandleDeath();
	}
}

void AVXCharacterBase::OnMoveSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AVXCharacterBase::HandleDeath()
{
	bIsDead = true;

	AbilitySystemComponent->AddLooseGameplayTag(VXTags::State_Dead);
	AbilitySystemComponent->CancelAllAbilities();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	OnDeath.Broadcast(this);
}
