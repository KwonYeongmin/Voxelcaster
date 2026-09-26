// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VXGameMode.generated.h"

class UVXWaveManager;

/** 기본 게임 모드. 웨이브 매니저를 소유한다. 게임 상태 전환(보상 선택, 결과)은 이후 spec에서 추가한다. */
UCLASS()
class VOXELCASTER_API AVXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVXGameMode();

	UVXWaveManager* GetWaveManager() const { return WaveManager; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	TObjectPtr<UVXWaveManager> WaveManager;
};
