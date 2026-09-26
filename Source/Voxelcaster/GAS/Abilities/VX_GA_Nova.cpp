// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VX_GA_Nova.h"
#include "Character/VXCharacterBase.h"
#include "Combat/VXNovaField.h"
#include "Engine/World.h"
#include "GAS/VXGameplayTags.h"

UVX_GA_Nova::UVX_GA_Nova()
{
	// 틱당 피해. 즉시 1회 + 0.5초마다 → 2초 동안 5회, 총 50 (제안)
	Damage = 10.f;
	CooldownDuration = 4.f;
	CastVibrationIntensity = 0.6f;
	CastVibrationDuration = 0.1f;
}

FGameplayTag UVX_GA_Nova::GetCooldownTag() const
{
	return VXTags::Cooldown_Nova;
}

void UVX_GA_Nova::ExecuteSkill(AVXCharacterBase* Caster)
{
	const FTransform SpawnTransform(Caster->GetActorLocation());
	AVXNovaField* Field = Caster->GetWorld()->SpawnActorDeferred<AVXNovaField>(
		AVXNovaField::StaticClass(), SpawnTransform, Caster, Caster, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Field)
	{
		Field->Init(Caster, GetCooldownTag(), Radius, Damage, TickInterval, Duration);
		Field->FinishSpawning(SpawnTransform);
	}
}
