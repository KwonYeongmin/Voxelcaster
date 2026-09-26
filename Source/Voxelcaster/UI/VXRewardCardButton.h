// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "VXRewardCardButton.generated.h"

class UBorder;
class UTextBlock;
class UVX_VM_RewardCard;

/**
 * 보상 카드 버튼 (DES-UI-REWARD-001). WBP_RewardCard의 부모 클래스. CommonUI 버튼이다.
 * - 값은 UVX_VM_RewardCard에 있다. WBP는 Viewmodels 패널에 VX_VM_RewardCard(Manual)을 추가해 연결한다.
 *   강조 연출은 뷰모델의 bHighlighted에 바인딩한다.
 * - WBP가 없으면 C++ 기본 위젯 트리를 만들고 직접 표시·강조한다.
 */
UCLASS()
class VOXELCASTER_API UVXRewardCardButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	/** 카드 내용을 뷰모델에 채우고 이 위젯에 연결한다 */
	void SetCard(const FVXUpgradeCard& InCard, int32 InIndex, UVX_VM_RewardCard* InViewModel);
	const FVXUpgradeCard& GetCard() const { return Card; }
	int32 GetCardIndex() const { return CardIndex; }

	/** 강조 상태 (포커스·호버) */
	void SetHighlighted(bool bInHighlighted);

private:
	void BuildDefaultTree();

	UPROPERTY(Transient)
	TObjectPtr<UVX_VM_RewardCard> ViewModel;

	bool bBuiltInCode = false;

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
