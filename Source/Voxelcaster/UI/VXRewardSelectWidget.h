// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "VXRewardSelectWidget.generated.h"

class UCommonButtonBase;
class UHorizontalBox;
class UTextBlock;
class UVXRewardCardButton;
class UVX_VM_Reward;

/**
 * 보상 선택 화면 (DES-UI-REWARD-001). WBP_RewardSelect의 부모 클래스. CommonUI 활성화 위젯.
 * - 열리면 가운데 카드에 포커스, 좌우 입력이나 마우스 호버로 이동
 * - 포커스된 카드는 확대·강조, 하단에 그 스킬의 현재 빌드와 미리보기
 * - 뒤로가기로 닫을 수 없다. 확인 후 0.3초 연출 뒤 닫히고 선택이 적용된다
 *
 * WBP_RewardSelect 만들기:
 * - 카드 위젯(WBP_RewardCard, 부모 VXRewardCardButton) 3개를 이름 Card0, Card1, Card2로 배치한다
 * - Viewmodels 패널에 VX_VM_Reward(Manual)을 추가하고 TitleText·HintText·BuildText를 바인딩한다
 * WBP가 없으면 C++ 기본 위젯 트리를 만든다.
 */
UCLASS()
class VOXELCASTER_API UVXRewardSelectWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	/** 카드를 채운다. 열기 전에 호출한다. */
	void SetChoices(const TArray<FVXUpgradeCard>& InChoices, int32 WaveIndex);

	/** 가운데 카드에 포커스를 준다 */
	void FocusDefaultCard();

protected:
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** WBP에서 이름이 같은 카드 위젯이 자동으로 연결된다 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVXRewardCardButton> Card0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVXRewardCardButton> Card1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVXRewardCardButton> Card2;

private:
	void BuildDefaultTree();
	void HandleCardFocused(UCommonButtonBase* Button);
	void HandleCardClicked(UCommonButtonBase* Button);
	void UpdateBuildText(const FVXUpgradeCard& Card);
	void FinishSelection();

	UPROPERTY(Transient)
	TObjectPtr<UVX_VM_Reward> ViewModel;

	bool bBuiltInCode = false;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> CardRow;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BuildText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	/** 이번에 쓰는 카드 위젯 (WBP의 Card0~2 또는 기본 트리에서 만든 카드) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UVXRewardCardButton>> Cards;

	/** 확인 후 닫힐 때까지 남은 시간. 음수면 대기 아님 */
	float ConfirmTimer = -1.f;
	int32 ChosenIndex = INDEX_NONE;

	static constexpr float ConfirmDelay = 0.3f;
};
