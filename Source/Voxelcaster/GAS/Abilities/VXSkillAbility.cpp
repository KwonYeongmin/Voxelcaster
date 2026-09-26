// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VXSkillAbility.h"
#include "Character/VXCharacterBase.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GAS/VXAbilitySystemComponent.h"
#include "GAS/VXHitContext.h"
#include "Modifier/VXModifierComponent.h"

UVXSkillAbility::UVXSkillAbility()
{
	bBlockedWhileDashing = true;
}

UVXModifierComponent* UVXSkillAbility::GetModifierComponent() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();
	return nullptr != Avatar ? Avatar->FindComponentByClass<UVXModifierComponent>() : nullptr;
}

float UVXSkillAbility::GetEffectiveCooldown() const
{
	const UVXModifierComponent* Modifiers = GetModifierComponent();
	return nullptr != Modifiers ? Modifiers->GetModifiedCooldown(GetCooldownTag(), CooldownDuration) : CooldownDuration;
}

void UVXSkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 쿨다운 시작
	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AVXCharacterBase* Caster = Cast<AVXCharacterBase>(ActorInfo->AvatarActor.Get()))
	{
		ExecuteSkill(Caster);
	}

	// 스킬은 즉발이다. 쿨다운은 GE가 관리하므로 어빌리티는 바로 종료한다.
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UVXSkillAbility::GatherHostilesInRadius(const AVXCharacterBase* Caster, const FVector& Center, float Radius, TArray<AVXCharacterBase*>& OutTargets) const
{
	UWorld* World = Caster ? Caster->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VoxelSkillOverlap), false, Caster);
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AVXCharacterBase* Other = Cast<AVXCharacterBase>(Overlap.GetActor());
		if (Other && false == Other->IsDead() && Caster->IsHostileTo(Other))
		{
			OutTargets.AddUnique(Other);
		}
	}
}

void UVXSkillAbility::ApplyHit(AVXCharacterBase* Caster, AVXCharacterBase* Target, const FVector& HitLocation, const FVector& Direction, float DamageScale) const
{
	if (nullptr == Caster || nullptr == Target)
	{
		return;
	}

	FVXHitContext Context;
	Context.SkillTag = GetCooldownTag();
	Context.Source = Caster;
	Context.Target = Target;
	Context.Location = HitLocation;
	Context.Direction = Direction;
	Context.Damage = Damage * DamageScale;
	Context.bIsDerived = false;

	if (UVXAbilitySystemComponent* ASC = Caster->GetVoxelAbilitySystemComponent())
	{
		ASC->ApplySkillHit(Context);
	}
}
