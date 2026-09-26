// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/VXDataSettings.h"
#include "Engine/DataTable.h"

UVXDataSettings::UVXDataSettings()
{
	// 기본값. DefaultGame.ini 값이 있으면 그 값을 쓴다.
	WaveTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_Waves.DT_Waves")));
	EnemyTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_Enemies.DT_Enemies")));
	SkillTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_SkillRows.DT_SkillRows")));
	ModifierTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_Modifiers.DT_Modifiers")));
	TextTableKor = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_UIText_Kor.DT_UIText_Kor")));
	TextTableEng = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_UIText_Eng.DT_UIText_Eng")));
}
