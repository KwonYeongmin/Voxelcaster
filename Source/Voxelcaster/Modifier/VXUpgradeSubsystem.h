// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Modifier/VXModifierComponent.h"
#include "VXUpgradeSubsystem.generated.h"

/** 보상 카드 한 장: (스킬, 모디파이어)와 장착 후 스택 수 (DES-REWARD-001) */
USTRUCT(BlueprintType)
struct FVXUpgradeCard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly)
	EVXModifierType Modifier = EVXModifierType::Explode;

	/** 이 카드를 고르면 되는 스택 수 */
	UPROPERTY(BlueprintReadOnly)
	int32 ResultStack = 1;

	/** 이미 장착한 모디파이어를 강화하는 카드인지 */
	bool IsStackUpgrade() const { return ResultStack > 1; }

	/** 표시용: "MagicBolt + Explode (2)" */
	FString GetLabel() const;
	/** 표시용 효과 설명 */
	FString GetDescription() const;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FVXChoicesReadySignature, const TArray<FVXUpgradeCard>& /*Choices*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FVXChoiceAppliedSignature, const FVXUpgradeCard& /*Card*/);

/**
 * 보상 카드 추첨과 장착 (SPC-UPGRADE-001). UI를 참조하지 않고 델리게이트로만 알린다.
 * - 후보: 모든 (스킬, 모디파이어) 중 장착 가능한 조합 (효과 없음·슬롯 가득·최대 스택 제외)
 * - 서로 다른 3장을 균등 확률로 뽑고, 후보가 충분하면 최소 2종 스킬이 나오게 한다.
 */
UCLASS()
class VOXELCASTER_API UVXUpgradeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 후보를 추첨해 OnChoicesReady를 발행한다. 후보가 없으면 false (보상 건너뜀) */
	bool DrawChoices(UVXModifierComponent* Modifiers, int32 Count = 3);

	/** 현재 후보 중 하나를 장착한다. 이미 골랐거나 잘못된 인덱스면 false */
	bool ApplyChoice(int32 Index);

	const TArray<FVXUpgradeCard>& GetChoices() const { return Choices; }
	bool HasPendingChoices() const { return false == Choices.IsEmpty(); }

	FVXChoicesReadySignature OnChoicesReady;
	FVXChoiceAppliedSignature OnChoiceApplied;

private:
	TArray<FVXUpgradeCard> Choices;
	TWeakObjectPtr<UVXModifierComponent> ChoiceTarget;
};
