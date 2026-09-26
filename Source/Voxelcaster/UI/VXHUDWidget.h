// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "VXHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UVXHudViewModel;
class UVXSkillSlotViewModel;

/** C++ 기본 위젯 트리의 스킬 슬롯 위젯 묶음 */
USTRUCT()
struct FVXHUDSkillSlotWidgets
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> CooldownBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CooldownText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ModifierText;
};

/**
 * 전투 HUD (DES-UI-HUD-001). WBP_HUD의 부모 클래스.
 * - 값은 UVXHudViewModel에 있다. 플레이어 컨트롤러가 채운다.
 * - WBP_HUD: Viewmodels 패널에 VXHudViewModel(Manual)을 추가하고 View Bindings로 연결한다.
 * - WBP가 없으면 C++ 기본 위젯 트리를 만들고 뷰모델 값을 직접 읽어 표시한다.
 */
UCLASS()
class VOXELCASTER_API UVXHUDWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetViewModel(UVXHudViewModel* InViewModel);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BuildDefaultTree();
	void RefreshDefaultTree();
	void RefreshSlot(FVXHUDSkillSlotWidgets& Widgets, const UVXSkillSlotViewModel* SlotViewModel) const;

	UPROPERTY(Transient)
	TObjectPtr<UVXHudViewModel> ViewModel;

	/** WBP 없이 C++로 위젯 트리를 만들었는지 */
	bool bBuiltInCode = false;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EnemyText;

	UPROPERTY(Transient)
	TArray<FVXHUDSkillSlotWidgets> SlotWidgets;
};
