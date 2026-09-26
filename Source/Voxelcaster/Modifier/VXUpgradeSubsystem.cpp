// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modifier/VXUpgradeSubsystem.h"
#include "GAS/VXGameplayTags.h"
#include "UI/VXText.h"
#include "Voxelcaster.h"

FString FVXUpgradeCard::GetLabel() const
{
	return FString::Printf(TEXT("%s + %s (%d)"), *UVXModifierComponent::GetSkillName(SkillTag),
		*UVXModifierComponent::GetModifierName(Modifier), ResultStack);
}

FString FVXUpgradeCard::GetDescription() const
{
	const int32 N = ResultStack;
	switch (Modifier)
	{
	case EVXModifierType::Pierce:  return VXText::Format(TEXT("Desc.Pierce"), { N });
	case EVXModifierType::Split:   return VXText::Format(TEXT("Desc.Split"), { N + 1 });
	case EVXModifierType::Explode: return VXText::Format(TEXT("Desc.Explode"), { FString::Printf(TEXT("%.1f"), 1.5f + 0.5f * (N - 1)) });
	case EVXModifierType::Chain:   return VXText::Format(TEXT("Desc.Chain"), { N });
	case EVXModifierType::Haste:   return VXText::Format(TEXT("Desc.Haste"), { 15 * N });
	}
	return FString();
}

bool UVXUpgradeSubsystem::DrawChoices(UVXModifierComponent* Modifiers, int32 Count)
{
	Choices.Reset();
	ChoiceTarget = Modifiers;
	if (nullptr == Modifiers)
	{
		return false;
	}

	// 후보 풀: 장착 가능한 모든 (스킬, 모디파이어)
	const FGameplayTag Skills[] = { VoxelTags::Cooldown_MagicBolt, VoxelTags::Cooldown_Nova, VoxelTags::Cooldown_BladeSweep };
	TArray<FVXUpgradeCard> Pool;
	for (const FGameplayTag& Skill : Skills)
	{
		for (int32 i = 0; i <= static_cast<int32>(EVXModifierType::Haste); ++i)
		{
			const EVXModifierType Type = static_cast<EVXModifierType>(i);
			if (Modifiers->CanAddModifier(Skill, Type))
			{
				FVXUpgradeCard Card;
				Card.SkillTag = Skill;
				Card.Modifier = Type;
				Card.ResultStack = Modifiers->GetStack(Skill, Type) + 1;
				Pool.Add(Card);
			}
		}
	}

	if (Pool.IsEmpty())
	{
		UE_LOG(LogVoxel, Log, TEXT("Upgrade: no available cards, skipping reward"));
		return false;
	}

	// 셔플 후 앞에서부터. 후보가 충분하면 최소 2종 스킬이 나오도록 몇 번 다시 뽑는다.
	const int32 Pick = FMath::Min(Count, Pool.Num());
	for (int32 Attempt = 0; Attempt < 8; ++Attempt)
	{
		for (int32 i = Pool.Num() - 1; i > 0; --i)
		{
			Pool.Swap(i, FMath::RandRange(0, i));
		}

		TSet<FGameplayTag> SkillSet;
		for (int32 i = 0; i < Pick; ++i)
		{
			SkillSet.Add(Pool[i].SkillTag);
		}
		if (SkillSet.Num() >= 2 || Pick < 2)
		{
			break;
		}
	}

	for (int32 i = 0; i < Pick; ++i)
	{
		Choices.Add(Pool[i]);
	}

	OnChoicesReady.Broadcast(Choices);
	return true;
}

bool UVXUpgradeSubsystem::ApplyChoice(int32 Index)
{
	UVXModifierComponent* Modifiers = ChoiceTarget.Get();
	if (nullptr == Modifiers || false == Choices.IsValidIndex(Index))
	{
		return false;
	}

	const FVXUpgradeCard Card = Choices[Index];
	Choices.Reset(); // 중복 선택 방지

	if (false == Modifiers->AddModifier(Card.SkillTag, Card.Modifier))
	{
		return false;
	}

	UE_LOG(LogVoxel, Log, TEXT("Upgrade: picked %s"), *Card.GetLabel());
	OnChoiceApplied.Broadcast(Card);
	return true;
}
