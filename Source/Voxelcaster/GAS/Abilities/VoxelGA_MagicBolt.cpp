// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VoxelGA_MagicBolt.h"
#include "Character/VoxelCharacterBase.h"
#include "Combat/VoxelProjectile.h"
#include "Engine/World.h"
#include "GAS/VoxelGameplayTags.h"

UVoxelGA_MagicBolt::UVoxelGA_MagicBolt()
{
	Damage = 20.f;
	CooldownDuration = 0.4f;
}

FGameplayTag UVoxelGA_MagicBolt::GetCooldownTag() const
{
	return VoxelTags::Cooldown_MagicBolt;
}

void UVoxelGA_MagicBolt::ExecuteSkill(AVXCharacterBase* Caster)
{
	UWorld* World = Caster->GetWorld();
	const FVector Direction = Caster->GetAimDirection();
	const FVector SpawnLocation = Caster->GetActorLocation() + Direction * 60.f;

	AVoxelProjectile* Projectile = World->SpawnActorDeferred<AVoxelProjectile>(
		AVoxelProjectile::StaticClass(), FTransform(Direction.Rotation(), SpawnLocation), Caster, Caster,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		Projectile->Init(Caster, GetCooldownTag(), Damage, ProjectileSpeed, MaxRange);
		Projectile->FinishSpawning(FTransform(Direction.Rotation(), SpawnLocation));
	}
}
