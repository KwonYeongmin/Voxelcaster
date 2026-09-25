// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VXNovaField.generated.h"

class AVXCharacterBase;

/**
 * 노바의 지속 피해 장판. 시전자를 따라다니며 반경 안의 적에게 일정 간격으로 피해를 준다.
 * 틱마다 적 한 명당 명중 이벤트(ApplySkillHit)가 한 번씩 발생한다. (모디파이어 훅)
 */
UCLASS()
class VOXELCASTER_API AVXNovaField : public AActor
{
	GENERATED_BODY()

public:
	AVXNovaField();

	/** 스폰 직후(BeginPlay 전) 호출한다. */
	void Init(AVXCharacterBase* InCaster, const FGameplayTag& InSkillTag, float InRadius, float InDamagePerTick, float InTickInterval, float InDuration);

protected:
	virtual void BeginPlay() override;

private:
	void ApplyTick();

	TWeakObjectPtr<AVXCharacterBase> Caster;
	FGameplayTag SkillTag;
	float Radius = 300.f;
	float DamagePerTick = 10.f;
	float TickInterval = 0.5f;
	float Duration = 2.f;

	FTimerHandle TickTimer;
};
