// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/VXGameMode.h"
#include "Character/VXPlayerCharacter.h"
#include "Player/VXPlayerController.h"
#include "Wave/VXWaveManager.h"

AVXGameMode::AVXGameMode()
{
	DefaultPawnClass = AVXPlayerCharacter::StaticClass();
	PlayerControllerClass = AVXPlayerController::StaticClass();

	WaveManager = CreateDefaultSubobject<UVXWaveManager>(TEXT("WaveManager"));
}
