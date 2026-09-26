// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/VXGameMode.h"
#include "Character/VXPlayerCharacter.h"
#include "Player/VXPlayerController.h"
#include "Wave/VXWaveManager.h"
#include "Data/VXDataManager.h"

AVXGameMode::AVXGameMode()
{
	DefaultPawnClass = AVXPlayerCharacter::StaticClass();
	PlayerControllerClass = AVXPlayerController::StaticClass();

	WaveManager = CreateDefaultSubobject<UVXWaveManager>(TEXT("WaveManager"));
}

void AVXGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	// 웨이브 매니저·적·스킬보다 먼저 읽어 두고, 빠진 데이터는 여기서 한 번에 로그로 알린다.
	if (UVXDataManager* Data = UVXDataManager::Get())
	{
		Data->LoadAll();
	}

	Super::InitGame(MapName, Options, ErrorMessage);
}
