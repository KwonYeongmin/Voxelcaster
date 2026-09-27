// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VXCharacterBase.h"
#include "VXPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UVXModifierComponent;

UCLASS()
class VOXELCASTER_API AVXPlayerCharacter : public AVXCharacterBase
{
	GENERATED_BODY()

public:
	AVXPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayAttackBounce(float Strength) override;

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

	// ---- 공격 출렁임 ("띠용"): 위로 늘었다가 감쇠 진동하며 돌아온다. 부피는 유지 (키가 늘면 몸통이 가늘어짐) ----

	/** 최대 늘어나는 비율 (0.22 = 키 22%) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel|Bounce")
	float AttackBounceAmount = 0.22f;

	/** 출렁이는 빠르기 (Hz) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel|Bounce")
	float AttackBounceFrequency = 6.f;

	/** 줄어드는 빠르기. 클수록 빨리 멈춘다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel|Bounce")
	float AttackBounceDamping = 9.f;

	/** 이 시간(초)이 지나면 원래 크기로 되돌린다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel|Bounce")
	float AttackBounceDuration = 0.45f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	void TickAttackBounce(float DeltaSeconds);
	void ApplyMeshSquash(float ScaleXY, float ScaleZ);

	/** 출렁임 경과 시간. 음수면 진행 중 아님 */
	float BounceElapsed = -1.f;
	float BounceStrength = 1.f;

	/** 출렁임 전 메시의 원래 위치·크기 (BP에서 맞춘 값) */
	FVector MeshBaseLocation = FVector::ZeroVector;
	FVector MeshBaseScale = FVector::OneVector;
	bool bMeshBaseCaptured = false;

	/** 화면 흔들림이 끝나는 실제 시간. 0이면 진행 중 아님 */
	double ShakeEndRealTime = 0.0;
	FVector BaseSocketOffset = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	/** 스킬별 모디파이어 장착과 효과 실행 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Modifier")
	TObjectPtr<UVXModifierComponent> ModifierComponent;

	/** 카메라와 캐릭터 사이를 가리는 벽을 화면에서 숨긴다 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<class UVXCameraOcclusionComponent> CameraOcclusion;
};
