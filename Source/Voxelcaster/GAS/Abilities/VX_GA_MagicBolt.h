// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/VXSkillAbility.h"
#include "VX_GA_MagicBolt.generated.h"

class AVXProjectile;

/** 매직 볼트: 조준 방향 직선 투사체 1발, 피해 20, 쿨다운 0.4초. 주력 평타. (DES-SKILL-001) */
UCLASS()
class VOXELCASTER_API UVX_GA_MagicBolt : public UVXSkillAbility
{
	GENERATED_BODY()

public:
	UVX_GA_MagicBolt();

protected:
	virtual void ExecuteSkill(AVXCharacterBase* Caster) override;
	virtual FGameplayTag GetCooldownTag() const override;

	/** 투사체 속도 (cm/s). 20 m/s = 2000 */
	UPROPERTY(EditDefaultsOnly, Category = "MagicBolt")
	float ProjectileSpeed = 2000.f;

	/** 최대 사거리 (cm). 15 m = 1500 */
	UPROPERTY(EditDefaultsOnly, Category = "MagicBolt")
	float MaxRange = 1500.f;
};
