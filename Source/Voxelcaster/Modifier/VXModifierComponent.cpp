// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modifier/VXModifierComponent.h"
#include "Character/VoxelCharacterBase.h"
#include "Combat/VoxelProjectile.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/VoxelAbilitySystemComponent.h"
#include "GAS/VoxelGameplayTags.h"

int32 FVXModifierSlots::GetStack(EVXModifierType Type) const
{
	int32 Count = 0;
	for (const EVXModifierType Slot : Slots)
	{
		if (Slot == Type)
		{
			++Count;
		}
	}
	return Count;
}

UVXModifierComponent::UVXModifierComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVXModifierComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AVXCharacterBase* Owner = GetOwnerCharacter())
	{
		if (UVoxelAbilitySystemComponent* ASC = Owner->GetVoxelAbilitySystemComponent())
		{
			ASC->OnSkillHit.AddUObject(this, &UVXModifierComponent::HandleSkillHit);
			bBoundToHits = true;
		}
	}
}

AVXCharacterBase* UVXModifierComponent::GetOwnerCharacter() const
{
	return Cast<AVXCharacterBase>(GetOwner());
}

// ---------------------------------------------------------------------------
// 장착
// ---------------------------------------------------------------------------

bool UVXModifierComponent::IsModifierValidForSkill(const FGameplayTag& SkillTag, EVXModifierType Type)
{
	// 관통은 투사체(매직 볼트)에만 효과가 있다. (DES-MOD-001)
	if (EVXModifierType::Pierce == Type)
	{
		return SkillTag == VoxelTags::Cooldown_MagicBolt;
	}
	return SkillTag.IsValid();
}

bool UVXModifierComponent::CanAddModifier(const FGameplayTag& SkillTag, EVXModifierType Type) const
{
	if (false == IsModifierValidForSkill(SkillTag, Type))
	{
		return false;
	}
	return GetUsedSlots(SkillTag) < MaxSlots && GetStack(SkillTag, Type) < MaxStack;
}

bool UVXModifierComponent::AddModifier(const FGameplayTag& SkillTag, EVXModifierType Type)
{
	if (false == CanAddModifier(SkillTag, Type))
	{
		return false;
	}

	Equipped.FindOrAdd(SkillTag).Slots.Add(Type);
	OnModifiersChanged.Broadcast();
	return true;
}

int32 UVXModifierComponent::GetStack(const FGameplayTag& SkillTag, EVXModifierType Type) const
{
	const FVXModifierSlots* Slots = Equipped.Find(SkillTag);
	return nullptr != Slots ? Slots->GetStack(Type) : 0;
}

int32 UVXModifierComponent::GetUsedSlots(const FGameplayTag& SkillTag) const
{
	const FVXModifierSlots* Slots = Equipped.Find(SkillTag);
	return nullptr != Slots ? Slots->Slots.Num() : 0;
}

const FVXModifierSlots* UVXModifierComponent::FindSlots(const FGameplayTag& SkillTag) const
{
	return Equipped.Find(SkillTag);
}

void UVXModifierComponent::ResetModifiers()
{
	Equipped.Reset();
	OnModifiersChanged.Broadcast();
}

float UVXModifierComponent::GetModifiedCooldown(const FGameplayTag& SkillTag, float BaseCooldown) const
{
	const int32 Stack = GetStack(SkillTag, EVXModifierType::Haste);
	if (Stack <= 0)
	{
		return BaseCooldown;
	}
	// 덧셈 감소: 1스택 -15%, 2스택 -30%, 3스택 -45%
	return FMath::Max(BaseCooldown * (1.f - HasteReductionPerStack * Stack), MinCooldown);
}

FString UVXModifierComponent::GetModifierName(EVXModifierType Type)
{
	switch (Type)
	{
	case EVXModifierType::Pierce:  return TEXT("Pierce");
	case EVXModifierType::Split:   return TEXT("Split");
	case EVXModifierType::Explode: return TEXT("Explode");
	case EVXModifierType::Chain:   return TEXT("Chain");
	case EVXModifierType::Haste:   return TEXT("Haste");
	}
	return TEXT("?");
}

FString UVXModifierComponent::GetSkillName(const FGameplayTag& SkillTag)
{
	if (SkillTag == VoxelTags::Cooldown_MagicBolt)
	{
		return TEXT("MagicBolt");
	}
	if (SkillTag == VoxelTags::Cooldown_Nova)
	{
		return TEXT("Nova");
	}
	if (SkillTag == VoxelTags::Cooldown_BladeSweep)
	{
		return TEXT("BladeSweep");
	}
	return SkillTag.ToString();
}

// ---------------------------------------------------------------------------
// 명중 처리
// ---------------------------------------------------------------------------

void UVXModifierComponent::HandleSkillHit(const FVoxelHitContext& Context)
{
	// 파생 효과는 다른 모디파이어를 다시 발동하지 않는다. (무한 연쇄 방지)
	if (Context.bIsDerived)
	{
		return;
	}

	const FVXModifierSlots* Slots = Equipped.Find(Context.SkillTag);
	if (nullptr == Slots || Slots->Slots.IsEmpty())
	{
		return;
	}

	// 모든 파생 효과는 원본 명중 시점의 정보로 대상을 정한다.
	if (const int32 Stack = Slots->GetStack(EVXModifierType::Explode))
	{
		ApplyExplode(Context, Stack);
	}
	if (const int32 Stack = Slots->GetStack(EVXModifierType::Chain))
	{
		ApplyChain(Context, Stack);
	}
	if (const int32 Stack = Slots->GetStack(EVXModifierType::Split))
	{
		ApplySplit(Context, Stack);
	}
}

void UVXModifierComponent::ApplyDerivedHit(const FVoxelHitContext& Source, AVXCharacterBase* Target, const FVector& Location, float Damage) const
{
	const AVXCharacterBase* Owner = GetOwnerCharacter();
	UVoxelAbilitySystemComponent* ASC = nullptr != Owner ? Owner->GetVoxelAbilitySystemComponent() : nullptr;
	if (nullptr == ASC || nullptr == Target)
	{
		return;
	}

	FVoxelHitContext Derived = Source;
	Derived.Target = Target;
	Derived.Location = Location;
	Derived.Damage = Damage;
	Derived.bIsDerived = true;
	ASC->ApplySkillHit(Derived);
}

void UVXModifierComponent::GatherHostiles(const FVector& Center, float Radius, TArray<AVXCharacterBase*>& OutTargets) const
{
	const AVXCharacterBase* Owner = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (nullptr == Owner || nullptr == World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VXModifierOverlap), false, Owner);
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius), Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AVXCharacterBase* Other = Cast<AVXCharacterBase>(Overlap.GetActor());
		if (Other && false == Other->IsDead() && Owner->IsHostileTo(Other))
		{
			OutTargets.AddUnique(Other);
		}
	}
}

void UVXModifierComponent::ApplyExplode(const FVoxelHitContext& Context, int32 Stack)
{
	const float Radius = ExplodeBaseRadius + ExplodeRadiusPerStack * (Stack - 1);
	const float Damage = Context.Damage * ExplodeDamageRatio;

	TArray<AVXCharacterBase*> Targets;
	GatherHostiles(Context.Location, Radius, Targets);

	// 원본에 맞은 적도 폭발 범위 안이면 폭발 피해를 추가로 받는다.
	for (AVXCharacterBase* Target : Targets)
	{
		ApplyDerivedHit(Context, Target, Target->GetActorLocation(), Damage);
	}

#if ENABLE_DRAW_DEBUG
	// 임시 이펙트: 폭발 (주황)
	DrawDebugSphere(GetWorld(), Context.Location, Radius, 16, FColor(255, 140, 0), false, 0.25f, 0, 3.f);
#endif
}

void UVXModifierComponent::ApplyChain(const FVoxelHitContext& Context, int32 Stack)
{
	const float Damage = Context.Damage * ChainDamageRatio;

	TSet<const AVXCharacterBase*> Visited;
	const AVXCharacterBase* Current = Cast<AVXCharacterBase>(Context.Target.Get());
	FVector From = nullptr != Current ? Current->GetActorLocation() : Context.Location;
	if (Current)
	{
		Visited.Add(Current);
	}

	// 직전에 맞은 적에서 가장 가까운 적으로 스택 수만큼 전이한다. 이미 맞은 적은 고르지 않는다.
	for (int32 Jump = 0; Jump < Stack; ++Jump)
	{
		TArray<AVXCharacterBase*> Candidates;
		GatherHostiles(From, ChainRange, Candidates);

		AVXCharacterBase* Nearest = nullptr;
		float NearestDistSq = TNumericLimits<float>::Max();
		for (AVXCharacterBase* Candidate : Candidates)
		{
			if (Visited.Contains(Candidate))
			{
				continue;
			}
			const float DistSq = FVector::DistSquared(From, Candidate->GetActorLocation());
			if (DistSq < NearestDistSq)
			{
				NearestDistSq = DistSq;
				Nearest = Candidate;
			}
		}

		if (nullptr == Nearest)
		{
			break;
		}

		const FVector To = Nearest->GetActorLocation();
#if ENABLE_DRAW_DEBUG
		// 임시 이펙트: 연쇄 번개 (하늘색)
		DrawDebugLine(GetWorld(), From, To, FColor(80, 200, 255), false, 0.2f, 0, 4.f);
#endif
		Visited.Add(Nearest);
		ApplyDerivedHit(Context, Nearest, To, Damage);
		From = To;
	}
}

void UVXModifierComponent::ApplySplit(const FVoxelHitContext& Context, int32 Stack)
{
	AVXCharacterBase* Owner = GetOwnerCharacter();
	UWorld* World = GetWorld();
	if (nullptr == Owner || nullptr == World)
	{
		return;
	}

	LiveSplitProjectiles.RemoveAll([](const TWeakObjectPtr<AVoxelProjectile>& Projectile) { return false == Projectile.IsValid(); });

	const int32 Count = Stack + 1;
	const FVector Base = Context.Direction.IsNearlyZero() ? Owner->GetActorForwardVector() : Context.Direction.GetSafeNormal2D();
	const float Step = 360.f / Count;

	for (int32 i = 0; i < Count; ++i)
	{
		// 성능 제한: 초과분은 생략한다.
		if (LiveSplitProjectiles.Num() >= MaxSplitProjectiles)
		{
			break;
		}

		const FVector Direction = Base.RotateAngleAxis(Step * i + Step * 0.5f, FVector::UpVector);
		const FVector Location = Context.Location + Direction * 40.f;
		const FTransform SpawnTransform(Direction.Rotation(), Location);

		AVoxelProjectile* Projectile = World->SpawnActorDeferred<AVoxelProjectile>(
			AVoxelProjectile::StaticClass(), SpawnTransform, Owner, Owner, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (nullptr == Projectile)
		{
			continue;
		}

		// 분열 투사체는 작은 노란 투사체. 원본 명중 대상은 다시 맞히지 않는다.
		Projectile->Init(Owner, Context.SkillTag, Context.Damage * SplitDamageRatio, SplitSpeed, SplitRange, FLinearColor(1.f, 0.9f, 0.2f));
		Projectile->SetDerived(true);
		Projectile->IgnoreTarget(Context.Target.Get());
		Projectile->SetActorScale3D(FVector(0.6f));
		Projectile->FinishSpawning(SpawnTransform);
		LiveSplitProjectiles.Add(Projectile);
	}
}

// ---------------------------------------------------------------------------
// 디버그 표시
// ---------------------------------------------------------------------------

void UVXModifierComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (false == bShowDebugInfo || nullptr == GEngine)
	{
		return;
	}

	const FGameplayTag Skills[] = { VoxelTags::Cooldown_MagicBolt, VoxelTags::Cooldown_Nova, VoxelTags::Cooldown_BladeSweep };
	FString Text = TEXT("BUILD");
	for (const FGameplayTag& Skill : Skills)
	{
		Text += FString::Printf(TEXT("\n  %s:"), *GetSkillName(Skill));
		const FVXModifierSlots* Slots = Equipped.Find(Skill);
		for (int32 i = 0; i < MaxSlots; ++i)
		{
			Text += (nullptr != Slots && Slots->Slots.IsValidIndex(i))
				? FString::Printf(TEXT(" [%s]"), *GetModifierName(Slots->Slots[i]))
				: FString(TEXT(" [ ]"));
		}
	}
	GEngine->AddOnScreenDebugMessage(7003, 0.f, FColor::Cyan, Text);
}
