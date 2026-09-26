// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/VXDataManager.h"
#include "Data/VXDataSettings.h"
#include "Data/VXEnemyData.h"
#include "Data/VXModifierData.h"
#include "Data/VXSkillData.h"
#include "Data/VXWaveData.h"
#include "Enemy/VXEnemyBase.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "UI/VXInputPrompt.h"
#include "UI/VXText.h"
#include "Voxelcaster.h"

namespace
{
	const UScriptStruct* GetExpectedRowStruct(EVXDataTable Type)
	{
		switch (Type)
		{
		case EVXDataTable::Waves:     return FVXWaveRow::StaticStruct();
		case EVXDataTable::Enemies:   return FVXEnemyRow::StaticStruct();
		case EVXDataTable::Skills:    return FVXSkillRow::StaticStruct();
		case EVXDataTable::Modifiers: return FVXModifierRow::StaticStruct();
		case EVXDataTable::TextKor:
		case EVXDataTable::TextEng:   return FVXTextRow::StaticStruct();
		case EVXDataTable::InputIcons: return FVXInputIconRow::StaticStruct();
		default:                      return nullptr;
		}
	}

	const TSoftObjectPtr<UDataTable>& GetTablePath(const UVXDataSettings& Settings, EVXDataTable Type)
	{
		switch (Type)
		{
		case EVXDataTable::Waves:     return Settings.WaveTable;
		case EVXDataTable::Enemies:   return Settings.EnemyTable;
		case EVXDataTable::Skills:    return Settings.SkillTable;
		case EVXDataTable::Modifiers: return Settings.ModifierTable;
		case EVXDataTable::TextKor:   return Settings.TextTableKor;
		case EVXDataTable::InputIcons: return Settings.InputIconTable;
		default:                      return Settings.TextTableEng;
		}
	}

	/** 코드가 행 이름으로 찾는 행들. 빠지면 코드 기본값을 쓰게 되므로 경고한다 */
	TArray<FName> GetRequiredRows(EVXDataTable Type)
	{
		switch (Type)
		{
		case EVXDataTable::Enemies:   return { TEXT("Runner"), TEXT("Shooter"), TEXT("Elite") };
		case EVXDataTable::Skills:    return { TEXT("MagicBolt"), TEXT("Nova"), TEXT("BladeSweep"), TEXT("Dash") };
		case EVXDataTable::Modifiers: return { TEXT("Pierce"), TEXT("Split"), TEXT("Explode"), TEXT("Chain"), TEXT("Haste") };
		case EVXDataTable::TextKor:
		case EVXDataTable::TextEng:
		{
			TArray<FName> Keys;
			VXText::GetBuiltInKeys(Keys);
			return Keys;
		}
		default:                      return {};
		}
	}
}

UVXDataManager* UVXDataManager::Get()
{
	return nullptr != GEngine ? GEngine->GetEngineSubsystem<UVXDataManager>() : nullptr;
}

void UVXDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 테이블은 처음 쓸 때 또는 LoadAll에서 읽는다. (엔진 시작 시점에는 읽지 않는다)
	Tables.SetNum(static_cast<int32>(EVXDataTable::Count));
	bTried.Init(false, static_cast<int32>(EVXDataTable::Count));
}

const TCHAR* UVXDataManager::GetTableLabel(EVXDataTable Type)
{
	switch (Type)
	{
	case EVXDataTable::Waves:     return TEXT("Waves");
	case EVXDataTable::Enemies:   return TEXT("Enemies");
	case EVXDataTable::Skills:    return TEXT("Skills");
	case EVXDataTable::Modifiers: return TEXT("Modifiers");
	case EVXDataTable::TextKor:   return TEXT("TextKor");
	case EVXDataTable::TextEng:   return TEXT("TextEng");
	case EVXDataTable::InputIcons: return TEXT("InputIcons");
	default:                      return TEXT("?");
	}
}

// ---------------------------------------------------------------------------
// 로드
// ---------------------------------------------------------------------------

const UDataTable* UVXDataManager::LoadTable(EVXDataTable Type)
{
	const int32 Index = static_cast<int32>(Type);
	bTried[Index] = true;
	Tables[Index] = nullptr;

	const TSoftObjectPtr<UDataTable>& Path = GetTablePath(*GetDefault<UVXDataSettings>(), Type);
	if (Path.IsNull() && IsOptional(Type))
	{
		return nullptr;
	}
	if (Path.IsNull())
	{
		UE_LOG(LogVX, Warning, TEXT("[Data] %s: not set in Project Settings > Voxelcaster Data (using defaults in code)"), GetTableLabel(Type));
		return nullptr;
	}

	UDataTable* Table = Path.LoadSynchronous();
	if (nullptr == Table && IsOptional(Type))
	{
		UE_LOG(LogVX, Log, TEXT("[Data] %s: '%s' not found (optional, skipped)"), GetTableLabel(Type), *Path.ToString());
		return nullptr;
	}
	if (nullptr == Table)
	{
		UE_LOG(LogVX, Warning, TEXT("[Data] %s: '%s' could not be loaded (using defaults in code)"), GetTableLabel(Type), *Path.ToString());
		return nullptr;
	}

	const UScriptStruct* Expected = GetExpectedRowStruct(Type);
	if (Table->GetRowStruct() != Expected)
	{
		UE_LOG(LogVX, Error, TEXT("[Data] %s: '%s' row struct is %s, expected %s (using defaults in code)"), GetTableLabel(Type), *Table->GetName(),
			nullptr != Table->GetRowStruct() ? *Table->GetRowStruct()->GetName() : TEXT("None"), *Expected->GetName());
		return nullptr;
	}

	Tables[Index] = Table;
	return Table;
}

const UDataTable* UVXDataManager::GetTable(EVXDataTable Type)
{
	const int32 Index = static_cast<int32>(Type);
	if (false == Tables.IsValidIndex(Index))
	{
		return nullptr;
	}
	if (false == bTried[Index])
	{
		LoadTable(Type);
	}
	return Tables[Index];
}

bool UVXDataManager::LoadAll()
{
	int32 Problems = 0;
	int32 Loaded = 0;
	for (int32 Index = 0; Index < static_cast<int32>(EVXDataTable::Count); ++Index)
	{
		const EVXDataTable Type = static_cast<EVXDataTable>(Index);
		if (nullptr == LoadTable(Type))
		{
			Problems += IsOptional(Type) ? 0 : 1;
			continue;
		}
		++Loaded;
		Problems += ValidateTable(Type);
	}

	UE_LOG(LogVX, Log, TEXT("[Data] Loaded %d / %d tables, %d problem(s)"), Loaded, static_cast<int32>(EVXDataTable::Count), Problems);
	return 0 == Problems;
}

// ---------------------------------------------------------------------------
// 검사
// ---------------------------------------------------------------------------

int32 UVXDataManager::ValidateTable(EVXDataTable Type)
{
	const UDataTable* Table = Tables[static_cast<int32>(Type)];
	if (nullptr == Table)
	{
		return 1;
	}

	int32 Problems = 0;
	const TCHAR* Label = GetTableLabel(Type);
	const TArray<FName> RowNames = Table->GetRowNames();

	for (const FName& Required : GetRequiredRows(Type))
	{
		if (false == RowNames.Contains(Required))
		{
			UE_LOG(LogVX, Warning, TEXT("[Data] %s: row '%s' missing (using default in code)"), Label, *Required.ToString());
			++Problems;
		}
	}

	switch (Type)
	{
	case EVXDataTable::Waves:
		if (RowNames.IsEmpty())
		{
			UE_LOG(LogVX, Warning, TEXT("[Data] Waves: no rows (using default waves in code)"));
			++Problems;
		}
		Table->ForeachRow<FVXWaveRow>(TEXT("VXData"), [&](const FName& Name, const FVXWaveRow& Row)
		{
			for (const FVXWaveSpawn& Spawn : Row.Spawns)
			{
				if (nullptr == Spawn.EnemyClass || Spawn.TotalCount <= 0)
				{
					UE_LOG(LogVX, Warning, TEXT("[Data] Waves: row '%s' has a spawn without enemy class or count"), *Name.ToString());
					++Problems;
				}
			}
		});
		break;

	case EVXDataTable::Enemies:
		Table->ForeachRow<FVXEnemyRow>(TEXT("VXData"), [&](const FName& Name, const FVXEnemyRow& Row)
		{
			if (Row.MaxHealth <= 0.f || Row.AttackInterval <= 0.f)
			{
				UE_LOG(LogVX, Warning, TEXT("[Data] Enemies: row '%s' needs MaxHealth > 0 and AttackInterval > 0"), *Name.ToString());
				++Problems;
			}
		});
		break;

	case EVXDataTable::Skills:
		Table->ForeachRow<FVXSkillRow>(TEXT("VXData"), [&](const FName& Name, const FVXSkillRow& Row)
		{
			if (Row.Cooldown <= 0.f)
			{
				UE_LOG(LogVX, Log, TEXT("[Data] Skills: row '%s' Cooldown is 0 (using default in code)"), *Name.ToString());
			}
		});
		break;

	case EVXDataTable::Modifiers:
		Table->ForeachRow<FVXModifierRow>(TEXT("VXData"), [&](const FName& Name, const FVXModifierRow& Row)
		{
			if (Row.BaseValue <= 0.f)
			{
				UE_LOG(LogVX, Warning, TEXT("[Data] Modifiers: row '%s' BaseValue must be > 0"), *Name.ToString());
				++Problems;
			}
		});
		break;

	default:
		break;
	}

	UE_LOG(LogVX, Log, TEXT("[Data] %s: '%s' %d rows"), Label, *Table->GetName(), RowNames.Num());
	return Problems;
}

// ---------------------------------------------------------------------------
// 조회
// ---------------------------------------------------------------------------

const FVXEnemyRow* UVXDataManager::FindEnemy(FName RowName)
{
	const UDataTable* Table = GetTable(EVXDataTable::Enemies);
	return nullptr != Table ? Table->FindRow<FVXEnemyRow>(RowName, TEXT("VXData"), false) : nullptr;
}

const FVXSkillRow* UVXDataManager::FindSkill(FName RowName)
{
	const UDataTable* Table = GetTable(EVXDataTable::Skills);
	return nullptr != Table ? Table->FindRow<FVXSkillRow>(RowName, TEXT("VXData"), false) : nullptr;
}

const FVXModifierRow* UVXDataManager::FindModifier(FName RowName)
{
	const UDataTable* Table = GetTable(EVXDataTable::Modifiers);
	return nullptr != Table ? Table->FindRow<FVXModifierRow>(RowName, TEXT("VXData"), false) : nullptr;
}

const UDataTable* UVXDataManager::GetTextTable(bool bKorean)
{
	return GetTable(bKorean ? EVXDataTable::TextKor : EVXDataTable::TextEng);
}
