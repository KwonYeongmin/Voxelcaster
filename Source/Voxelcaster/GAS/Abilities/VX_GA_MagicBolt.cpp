// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VX_GA_MagicBolt.h"
#include "Character/VXCharacterBase.h"
#include "Combat/VXProjectile.h"
#include "Engine/World.h"
#include "GAS/VXGameplayTags.h"
#include "Data/VXSkillData.h"
#include "Modifier/VXModifierComponent.h"
#include "Data/VXModifierData.h"

UVX_GA_MagicBolt::UVX_GA_MagicBolt()
{
	Damage = 20.f;
	CooldownDuration = 0.4f;
	// 연사하므로 출렁임은 약하게
	AttackBounceStrength = 0.5f;
	SkillRowName = TEXT("MagicBolt");
}

void UVX_GA_MagicBolt::ApplySkillRow(const FVXSkillRow& Row)
{
	Super::ApplySkillRow(Row);
	if (Row.ProjectileSpeed > 0.f)
	{
		ProjectileSpeed = Row.ProjectileSpeed;
	}
	if (Row.MaxRange > 0.f)
	{
		MaxRange = Row.MaxRange;
	}
}

FGameplayTag UVX_GA_MagicBolt::GetCooldownTag() const
{
	return VXTags::Cooldown_MagicBolt;
}

void UVX_GA_MagicBolt::ExecuteSkill(AVXCharacterBase* Caster)
{
	UWorld* World = Caster->GetWorld();
	const FVector Direction = Caster->GetAimDirection();
	const FVector SpawnLocation = Caster->GetActorLocation() + Direction * 60.f;

	AVXProjectile* Projectile = World->SpawnActorDeferred<AVXProjectile>(
		AVXProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation), Caster, Caster,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		Projectile->Init(Caster, GetCooldownTag(), Damage, ProjectileSpeed, MaxRange);
		if (const UVXModifierComponent* Modifiers = GetModifierComponent())
		{
			const int32 PierceStack = Modifiers->GetStack(GetCooldownTag(), EVXModifierType::Pierce);
			Projectile->SetPierceCount(PierceStack > 0 ? VXModifierData::Get(EVXModifierType::Pierce).GetCount(PierceStack) : 0);
		}
		Projectile->FinishSpawning(FTransform(Direction.Rotation(), SpawnLocation));
	}
}
