// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VXCharacterBase.h"
#include "VXPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UVXModifierComponent;

UCLASS()
class VOXELCASTER_API AVXPlayerCharacter : public AVXCharacterBase
{
	GENERATED_BODY()

public:
	AVXPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void GrantStartupAbilities() override;
	virtual void HandleDamaged(float Amount) override;

	/** 피격 히트스톱 (실제 시간 초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float HitStopOnDamaged = 0.08f;

	/** 피격 진동 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float DamagedVibrationIntensity = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float DamagedVibrationDuration = 0.15f;

	/** 피격 화면 흔들림: 세기(cm)와 시간(초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float ShakeAmplitude = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float ShakeDuration = 0.15f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 화면 흔들림이 끝나는 실제 시간. 0이면 진행 중 아님 */
	double ShakeEndRealTime = 0.0;
	FVector BaseSocketOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	/** 스킬별 모디파이어 장착과 효과 실행 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Modifier")
	TObjectPtr<UVXModifierComponent> ModifierComponent;

	/** 복셀 에셋 확정 전까지 쓰는 임시 큐브 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
