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

/**
 * 보상 선택 화면 (DES-UI-REWARD-001). CommonUI 활성화 위젯이며 위젯 트리를 코드로 만든다.
 * - 열리면 가운데 카드에 포커스, 좌우 입력(방향키·스틱·D패드)이나 마우스 호버로 이동
 * - 포커스된 카드는 확대·강조되고, 하단에 그 스킬의 현재 빌드를 보여준다
 * - 뒤로가기(Esc·B)로 닫을 수 없다. 반드시 1장을 골라야 한다
 * - 확인 후 0.3초 연출 뒤 닫히고 선택이 적용된다 (게임 일시정지 중에도 동작)
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
	virtual bool NativeOnHandleBackAction() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void HandleCardFocused(UCommonButtonBase* Button);
	void HandleCardClicked(UCommonButtonBase* Button);

	void UpdateBuildText(const FVXUpgradeCard& Card);
	void FinishSelection();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> CardRow;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BuildText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UVXRewardCardButton>> Cards;

	/** 확인 후 닫힐 때까지 남은 시간. 음수면 대기 아님 */
	float ConfirmTimer = -1.f;
	int32 ChosenIndex = INDEX_NONE;

	/** 확인 연출 시간 (초) */
	static constexpr float ConfirmDelay = 0.3f;
};
