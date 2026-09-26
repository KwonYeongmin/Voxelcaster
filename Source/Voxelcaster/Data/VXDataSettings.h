// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VXDataSettings.generated.h"

class UDataTable;

/**
 * 게임 데이터 테이블 목록. 프로젝트 세팅 → Game → Voxelcaster Data 에서 지정한다. (DefaultGame.ini에 저장)
 * 에셋 이름이나 위치를 바꿔도 여기만 다시 지정하면 된다. 코드는 UVXDataManager로만 테이블을 읽는다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Voxelcaster Data"))
class VOXELCASTER_API UVXDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UVXDataSettings();

	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** 웨이브 (행 구조체 VXWaveRow) */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXWaveRow"))
	TSoftObjectPtr<UDataTable> WaveTable;

	/** 적 능력치 (VXEnemyRow). 행: Runner, Shooter, Elite */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXEnemyRow"))
	TSoftObjectPtr<UDataTable> EnemyTable;

	/** 스킬 수치 (VXSkillRow). 행: MagicBolt, Nova, BladeSweep, Dash */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXSkillRow"))
	TSoftObjectPtr<UDataTable> SkillTable;

	/** 모디파이어 수치 (VXModifierRow). 행: Pierce, Split, Explode, Chain, Haste */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXModifierRow"))
	TSoftObjectPtr<UDataTable> ModifierTable;

	/** 화면 문구 한국어 (VXTextRow) */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXTextRow"))
	TSoftObjectPtr<UDataTable> TextTableKor;

	/** 화면 문구 영어 (VXTextRow) */
	UPROPERTY(Config, EditAnywhere, Category = "Tables", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXTextRow"))
	TSoftObjectPtr<UDataTable> TextTableEng;
};
