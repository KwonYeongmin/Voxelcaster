// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VoxelProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class AVXCharacterBase;

/**
 * 플레이어 스킬 투사체 (매직 볼트). 적대 캐릭터에 닿으면 ApplySkillHit로 피해를 주고 사라진다.
 * 벽에 막히거나 사거리를 넘으면 사라진다. 관통·분열 같은 모디파이어는 명중 이벤트로 붙는다.
 */
UCLASS()
class VOXELCASTER_API AVoxelProjectile : public AActor
{
	GENERATED_BODY()

public:
	AVoxelProjectile();

	/** 스폰 직후(BeginPlay 전) 호출한다. */
	void Init(AVXCharacterBase* InCaster, const FGameplayTag& InSkillTag, float InDamage, float InSpeed, float InMaxRange,
		const FLinearColor& InColor = FLinearColor(0.3f, 0.8f, 1.f));

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProjectileStopped(const FHitResult& ImpactResult);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	TWeakObjectPtr<AVXCharacterBase> Caster;
	FGameplayTag SkillTag;
	float Damage = 0.f;
	float Speed = 2000.f;
	float MaxRange = 1500.f;
};
