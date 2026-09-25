// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GAS/VoxelGameplayAbility.h"
#include "VoxelGA_Dash.generated.h"

/**
 * 대시. 스킬 시스템 밖의 이동 기능이며 모디파이어의 영향을 받지 않는다. (DES-CHAR-001)
 * 쿨다운 1초, 무적 0.2초, 이동 거리 4m / 0.2초.
 */
UCLASS()
class VOXELCASTER_API UVoxelGA_Dash : public UVoxelGameplayAbility
{
	GENERATED_BODY()

public:
	UVoxelGA_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual FGameplayTag GetCooldownTag() const override;

	/** 이동 거리 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float Distance = 400.f;

	/** 이동 지속 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float Duration = 0.2f;

	/** 무적 시간 (초), 대시 시작 시점부터 */
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float InvincibleTime = 0.2f;

private:
	UFUNCTION()
	void OnDashFinished();

	FGameplayAbilitySpecHandle CurrentHandle;
	const FGameplayAbilityActorInfo* CurrentActorInfo_Cached = nullptr;
	FGameplayAbilityActivationInfo CurrentActivationInfo_Cached;
};
