// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/VoxelSkillAbility.h"
#include "VX_GA_BladeSweep.generated.h"

/** 블레이드 스윕: 조준 방향 전방 120° 부채꼴(반경 2.5m)에 피해 30, 쿨다운 1.5초. 근접 견제. (DES-SKILL-001) */
UCLASS()
class VOXELCASTER_API UVX_GA_BladeSweep : public UVoxelSkillAbility
{
	GENERATED_BODY()

public:
	UVX_GA_BladeSweep();

protected:
	virtual void ExecuteSkill(AVXCharacterBase* Caster) override;
	virtual FGameplayTag GetCooldownTag() const override;

	/** 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "BladeSweep")
	float Radius = 250.f;

	/** 부채꼴 전체 각도 (도) */
	UPROPERTY(EditDefaultsOnly, Category = "BladeSweep")
	float ArcAngle = 120.f;
};
