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

	/** 게임 시작 시 데이터 테이블을 모두 읽고 검사한다 (UVXDataManager) */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UVXWaveManager* GetWaveManager() const { return WaveManager; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	TObjectPtr<UVXWaveManager> WaveManager;
};
