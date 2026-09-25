// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/VoxelGameMode.h"
#include "Character/VoxelPlayerCharacter.h"
#include "Player/VoxelPlayerController.h"
#include "Wave/VoxelWaveManager.h"

AVoxelGameMode::AVoxelGameMode()
{
	DefaultPawnClass = AVXPlayerCharacter::StaticClass();
	PlayerControllerClass = AVXPlayerController::StaticClass();

	WaveManager = CreateDefaultSubobject<UVXWaveManager>(TEXT("WaveManager"));
}
