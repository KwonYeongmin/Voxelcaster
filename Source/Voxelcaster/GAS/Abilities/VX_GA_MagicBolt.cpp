// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VX_GA_MagicBolt.h"
#include "Character/VXCharacterBase.h"
#include "Combat/VXProjectile.h"
#include "Engine/World.h"
#include "GAS/VXGameplayTags.h"
#include "Modifier/VXModifierComponent.h"

UVX_GA_MagicBolt::UVX_GA_MagicBolt()
{
	Damage = 20.f;
	CooldownDuration = 0.4f;
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
			Projectile->SetPierceCount(Modifiers->GetStack(GetCooldownTag(), EVXModifierType::Pierce));
		}
		Projectile->FinishSpawning(FTransform(Direction.Rotation(), SpawnLocation));
	}
}
