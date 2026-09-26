// Copyright Epic Games, Inc. All Rights Reserved.

#include "Feel/VXGameFeelSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "Player/VXPlayerController.h"

namespace
{
	TAutoConsoleVariable<int32> CVarHitStop(
		TEXT("Voxel.GameFeel.HitStop"), 1,
		TEXT("1: 처치·피격 시 히트스톱을 쓴다"));

	TAutoConsoleVariable<int32> CVarVibration(
		TEXT("Voxel.GameFeel.Vibration"), 1,
		TEXT("1: 게임패드 진동을 쓴다"));
}

UVXGameFeelSubsystem* UVXGameFeelSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = nullptr != WorldContext ? WorldContext->GetWorld() : nullptr;
	return nullptr != World ? World->GetSubsystem<UVXGameFeelSubsystem>() : nullptr;
}

void UVXGameFeelSubsystem::RequestHitStop(float Duration)
{
	UWorld* World = GetWorld();
	if (Duration <= 0.f || nullptr == World || CVarHitStop.GetValueOnGameThread() <= 0)
	{
		return;
	}

	// 겹치면 가장 긴 것 하나만. 같은 프레임의 다수 처치도 한 번으로 합쳐진다.
	const double Now = FPlatformTime::Seconds();
	HitStopEndTime = FMath::Max(HitStopEndTime, Now + Duration);

	if (AWorldSettings* Settings = World->GetWorldSettings())
	{
		Settings->SetTimeDilation(HitStopTimeDilation);
	}
}

void UVXGameFeelSubsystem::Tick(float DeltaTime)
{
	if (HitStopEndTime > 0.0 && FPlatformTime::Seconds() >= HitStopEndTime)
	{
		EndHitStop();
	}
}

void UVXGameFeelSubsystem::EndHitStop()
{
	HitStopEndTime = 0.0;
	if (UWorld* World = GetWorld())
	{
		if (AWorldSettings* Settings = World->GetWorldSettings())
		{
			Settings->SetTimeDilation(1.f);
		}
	}
}

void UVXGameFeelSubsystem::Deinitialize()
{
	// 월드가 끝날 때 느려진 채로 남지 않게 한다. (재시작 등)
	if (HitStopEndTime > 0.0)
	{
		EndHitStop();
	}
	Super::Deinitialize();
}

void UVXGameFeelSubsystem::PlayVibration(APlayerController* PlayerController, float Intensity, float Duration)
{
	const AVXPlayerController* VXController = Cast<AVXPlayerController>(PlayerController);
	if (nullptr == VXController || CVarVibration.GetValueOnGameThread() <= 0)
	{
		return;
	}

	// 진동은 게임패드를 쓰고 있을 때만 (DES-CTRL-001)
	if (EVXInputDevice::Gamepad != VXController->GetInputDevice())
	{
		return;
	}

	PlayerController->PlayDynamicForceFeedback(FMath::Clamp(Intensity, 0.f, 1.f), Duration, true, true, true, true);
}

TStatId UVXGameFeelSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVXGameFeelSubsystem, STATGROUP_Tickables);
}

ETickableTickType UVXGameFeelSubsystem::GetTickableTickType() const
{
	// 게임 월드에서만 (에디터 월드·CDO 제외)
	return IsTemplate() ? ETickableTickType::Never : ETickableTickType::Always;
}
