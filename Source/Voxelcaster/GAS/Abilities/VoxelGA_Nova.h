// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/VoxelSkillAbility.h"
#include "VoxelGA_Nova.generated.h"

/**
 * 노바: 자신 주변 원형 범위(반경 3m)에 지속 피해를 주는 장판을 깐다. 쿨다운 4초. 포위 탈출용. (DES-SKILL-001)
 * 장판은 시전자를 따라다니며 Duration 동안 TickInterval마다 Damage(틱당 피해)를 준다. 시전 즉시 1회 포함.
 */
UCLASS()
class VOXELCASTER_API UVoxelGA_Nova : public UVoxelSkillAbility
{
	GENERATED_BODY()

public:
	UVoxelGA_Nova();

protected:
	virtual void ExecuteSkill(AVXCharacterBase* Caster) override;
	virtual FGameplayTag GetCooldownTag() const override;

	/** 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Nova")
	float Radius = 300.f;

	/** 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Nova", meta = (Units = "s"))
	float Duration = 2.f;

	/** 피해 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Nova", meta = (Units = "s"))
	float TickInterval = 0.5f;
};
