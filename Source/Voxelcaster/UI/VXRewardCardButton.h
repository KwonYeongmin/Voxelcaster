// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "VXRewardCardButton.generated.h"

class UBorder;
class UTextBlock;

/**
 * 보상 카드 버튼 (DES-UI-REWARD-001). 위젯 트리를 코드로 만든다. (에디터 에셋 없이 동작)
 * 포커스(게임패드)나 호버(마우스)되면 확대·강조된다.
 */
UCLASS()
class VOXELCASTER_API UVXRewardCardButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetCard(const FVXUpgradeCard& InCard, int32 InIndex);
	const FVXUpgradeCard& GetCard() const { return Card; }
	int32 GetCardIndex() const { return CardIndex; }

	/** 강조 상태 (포커스·호버) */
	void SetHighlighted(bool bInHighlighted);

private:
	UPROPERTY(Transient)
	TObjectPtr<UBorder> CardBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SkillText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ModifierText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DescText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TagText;

	FVXUpgradeCard Card;
	int32 CardIndex = INDEX_NONE;
};
