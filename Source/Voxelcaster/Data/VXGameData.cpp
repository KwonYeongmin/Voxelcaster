// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/VXModifierData.h"
#include "Data/VXSkillData.h"
#include "Data/VXDataManager.h"

namespace
{
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
	UVXDataManager* Data = UVXDataManager::Get();
	return nullptr != Data ? Data->FindSkill(RowName) : nullptr;
}

const FVXModifierRow& VXModifierData::Get(EVXModifierType Type)
{
	if (UVXDataManager* Data = UVXDataManager::Get())
	{
		if (const FVXModifierRow* Row = Data->FindModifier(*UVXModifierComponent::GetModifierName(Type)))
		{
			return *Row;
		}
	}
	return GetDefaultModifierRow(Type);
}
