// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/VXModifierData.h"
#include "Data/VXSkillData.h"
#include "Voxelcaster.h"

namespace
{
	/**
	 * 데이터 테이블을 읽어 캐시한다. 행 구조체가 다르면 쓰지 않는다.
	 * 없으면 로그를 한 번만 남기고 nullptr (코드 기본값 사용).
	 */
	const UDataTable* LoadTable(const TCHAR* Path, const UScriptStruct* RowStruct, TWeakObjectPtr<const UDataTable>& Cached, bool& bTried)
	{
		if (const UDataTable* Table = Cached.Get())
		{
			return Table;
		}
		if (bTried)
		{
			return nullptr;
		}
		bTried = true;

		const TSoftObjectPtr<UDataTable> Soft{ FSoftObjectPath(Path) };
		const UDataTable* Table = Soft.LoadSynchronous();
		if (Table && Table->GetRowStruct() == RowStruct)
		{
			Cached = Table;
			return Table;
		}

		UE_LOG(LogVX, Log, TEXT("%s not found or wrong row struct: using defaults in code"), Path);
		return nullptr;
	}

	FVXModifierRow MakeModifierRow(float BaseValue, float PerStackValue, float DamageRatio = 0.f, float Range = 0.f, float Speed = 0.f)
	{
		FVXModifierRow Row;
		Row.BaseValue = BaseValue;
		Row.PerStackValue = PerStackValue;
		Row.DamageRatio = DamageRatio;
		Row.Range = Range;
		Row.Speed = Speed;
		return Row;
	}

	/** 코드 기본값 (DES-MOD-001) */
	const FVXModifierRow& GetDefaultModifierRow(EVXModifierType Type)
	{
		static const FVXModifierRow Pierce = MakeModifierRow(1.f, 1.f);
		static const FVXModifierRow Split = MakeModifierRow(2.f, 1.f, 0.4f, 600.f, 1500.f);
		static const FVXModifierRow Explode = MakeModifierRow(150.f, 50.f, 0.5f);
		static const FVXModifierRow Chain = MakeModifierRow(1.f, 1.f, 0.7f, 500.f);
		static const FVXModifierRow Haste = MakeModifierRow(0.15f, 0.15f);

		switch (Type)
		{
		case EVXModifierType::Pierce:  return Pierce;
		case EVXModifierType::Split:   return Split;
		case EVXModifierType::Explode: return Explode;
		case EVXModifierType::Chain:   return Chain;
		case EVXModifierType::Haste:   return Haste;
		}
		return Pierce;
	}
}

const FVXSkillRow* VXSkillData::Find(FName RowName)
{
	static TWeakObjectPtr<const UDataTable> Cached;
	static bool bTried = false;

	const UDataTable* Table = LoadTable(TEXT("/Game/Voxelcaster/Data/DT_Skills.DT_Skills"), FVXSkillRow::StaticStruct(), Cached, bTried);
	return nullptr != Table ? Table->FindRow<FVXSkillRow>(RowName, TEXT("VXSkill"), false) : nullptr;
}

const FVXModifierRow& VXModifierData::Get(EVXModifierType Type)
{
	static TWeakObjectPtr<const UDataTable> Cached;
	static bool bTried = false;

	if (const UDataTable* Table = LoadTable(TEXT("/Game/Voxelcaster/Data/DT_Modifiers.DT_Modifiers"), FVXModifierRow::StaticStruct(), Cached, bTried))
	{
		if (const FVXModifierRow* Row = Table->FindRow<FVXModifierRow>(*UVXModifierComponent::GetModifierName(Type), TEXT("VXModifier"), false))
		{
			return *Row;
		}
	}
	return GetDefaultModifierRow(Type);
}
