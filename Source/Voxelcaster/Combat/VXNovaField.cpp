// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/VXNovaField.h"
#include "Character/VXCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "GAS/VXHitContext.h"
#include "TimerManager.h"

AVXNovaField::AVXNovaField()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AVXNovaField::Init(AVXCharacterBase* InCaster, const FGameplayTag& InSkillTag, float InRadius, float InDamagePerTick, float InTickInterval, float InDuration)
{
	Caster = InCaster;
	SkillTag = InSkillTag;
	Radius = InRadius;
	DamagePerTick = InDamagePerTick;
	TickInterval = FMath::Max(InTickInterval, 0.05f);
	Duration = InDuration;
}

void AVXNovaField::BeginPlay()
{
	Super::BeginPlay();

	// 시전자를 따라다닌다.
	if (AVXCharacterBase* CasterCharacter = Caster.Get())
	{
		AttachToActor(CasterCharacter, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	// 시전 즉시 1회, 이후 TickInterval마다. 마지막 틱이 Duration 안에 들어가도록 수명을 조금 더 준다.
	ApplyTick();
	GetWorldTimerManager().SetTimer(TickTimer, this, &AVXNovaField::ApplyTick, TickInterval, true);
	SetLifeSpan(Duration + KINDA_SMALL_NUMBER);
}

void AVXNovaField::ApplyTick()
{
	AVXCharacterBase* SourceCharacter = Caster.Get();
	if (nullptr == SourceCharacter || SourceCharacter->IsDead())
	{
		Destroy();
		return;
	}

	const FVector Center = SourceCharacter->GetActorLocation();

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VXNovaField), false, SourceCharacter);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius), QueryParams);

	TArray<AVXCharacterBase*> Targets;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AVXCharacterBase* Other = Cast<AVXCharacterBase>(Overlap.GetActor());
		if (Other && false == Other->IsDead() && SourceCharacter->IsHostileTo(Other))
		{
			Targets.AddUnique(Other);
		}
	}

	UVXAbilitySystemComponent* ASC = SourceCharacter->GetVoxelAbilitySystemComponent();
	for (AVXCharacterBase* Target : Targets)
	{
		FVXHitContext Context;
		Context.SkillTag = SkillTag;
		Context.Source = SourceCharacter;
		Context.Target = Target;
		Context.Location = Target->GetActorLocation();
		Context.Direction = (Target->GetActorLocation() - Center).GetSafeNormal2D();
		Context.Damage = DamagePerTick;
		Context.bIsDerived = false;
		ASC->ApplySkillHit(Context);
	}

#if ENABLE_DRAW_DEBUG
	// 임시 이펙트: 나중에 나이아가라로 교체한다.
	DrawDebugCircle(GetWorld(), Center - FVector(0, 0, 80), Radius, 48, FColor::Cyan, false, TickInterval * 0.5f, 0, 6.f,
		FVector::ForwardVector, FVector::RightVector, false);
#endif
}
