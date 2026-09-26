// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VXGameFeelSubsystem.generated.h"

class APlayerController;

/**
 * 타격감 (DES-FEEL-001). 히트스톱과 게임패드 진동을 한곳에서 관리한다.
 * - 히트스톱: 전역 시간 배율을 잠깐 낮춘다. 겹치면 가장 긴 것 하나만 적용한다 (누적하지 않음).
 *   끝나는 시점은 실제 시간으로 잰다. (느려진 게임 시간으로 재면 끝나지 않는다)
 * - 진동: 현재 입력 장치가 게임패드일 때만 준다.
 * 콘솔 변수 VX.GameFeel.HitStop / VX.GameFeel.Vibration (0 = 끔)
 */
UCLASS()
class VOXELCASTER_API UVXGameFeelSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UVXGameFeelSubsystem* Get(const UObject* WorldContext);

	/** 히트스톱 요청 (실제 시간 초). 진행 중인 것보다 길 때만 늘린다 */
	void RequestHitStop(float Duration);

	/** 게임패드 진동. 키보드·마우스 사용 중이면 아무 일도 하지 않는다 */
	void PlayVibration(APlayerController* PlayerController, float Intensity, float Duration);

	// UTickableWorldSubsystem
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual void Deinitialize() override;

	/** 히트스톱 중 시간 배율 */
	static constexpr float HitStopTimeDilation = 0.05f;

private:
	void EndHitStop();

	/** 히트스톱이 끝나는 실제 시간 (FPlatformTime::Seconds). 0이면 진행 중 아님 */
	double HitStopEndTime = 0.0;
};
