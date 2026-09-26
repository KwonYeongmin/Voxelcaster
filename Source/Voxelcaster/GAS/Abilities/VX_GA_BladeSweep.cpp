// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VX_GA_BladeSweep.h"
#include "Character/VoxelCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "GAS/VoxelGameplayTags.h"

UVX_GA_BladeSweep::UVX_GA_BladeSweep()
{
	Damage = 30.f;
	CooldownDuration = 1.5f;
}

FGameplayTag UVX_GA_BladeSweep::GetCooldownTag() const
{
	return VoxelTags::Cooldown_BladeSweep;
}

void UVX_GA_BladeSweep::ExecuteSkill(AVXCharacterBase* Caster)
{
	const FVector Center = Caster->GetActorLocation();
	const FVector Aim = Caster->GetAimDirection();
	const float CosHalfArc = FMath::Cos(FMath::DegreesToRadians(ArcAngle * 0.5f));

	TArray<AVXCharacterBase*> Targets;
	GatherHostilesInRadius(Caster, Center, Radius, Targets);

	for (AVXCharacterBase* Target : Targets)
	{
		const FVector ToTarget = (Target->GetActorLocation() - Center).GetSafeNormal2D();
		if (FVector::DotProduct(Aim, ToTarget) >= CosHalfArc)
		{
			ApplyHit(Caster, Target, Target->GetActorLocation(), ToTarget);
		}
	}

#if ENABLE_DRAW_DEBUG
	// 임시 이펙트: 부채꼴 테두리. 나중에 나이아가라로 교체한다.
	const UWorld* World = Caster->GetWorld();
	const FVector Base = Center - FVector(0, 0, 80);
	const float HalfArc = ArcAngle * 0.5f;
	const int32 Segments = 16;
	FVector Prev = Base + Aim.RotateAngleAxis(-HalfArc, FVector::UpVector) * Radius;
	DrawDebugLine(World, Base, Prev, FColor::Orange, false, 0.15f, 0, 6.f);
	for (int32 i = 1; i <= Segments; ++i)
	{
		const float Angle = -HalfArc + ArcAngle * (static_cast<float>(i) / Segments);
		const FVector Next = Base + Aim.RotateAngleAxis(Angle, FVector::UpVector) * Radius;
		DrawDebugLine(World, Prev, Next, FColor::Orange, false, 0.15f, 0, 6.f);
		Prev = Next;
	}
	DrawDebugLine(World, Base, Prev, FColor::Orange, false, 0.15f, 0, 6.f);
#endif
}
