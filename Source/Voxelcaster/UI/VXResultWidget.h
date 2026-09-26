// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "VXResultWidget.generated.h"

class UCommonButtonBase;
class UTextBlock;
class UVXModifierComponent;
class UVX_VM_Result;

/** 결과 화면에 보여줄 한 판의 기록 */
struct FVXRunResult
{
	bool bVictory = false;
	int32 ReachedWave = 0;
	int32 TotalWaves = 0;
	int32 Kills = 0;
	float PlayTime = 0.f;
};

/**
 * 결과 화면 (DES-UI-MENU-001). WBP_Result의 부모 클래스. CommonUI 활성화 위젯.
 * 승리/패배, 도달 웨이브, 처치 수, 플레이 시간, 최종 빌드, 재시작(기본 포커스)·종료. 닫을 수 없다.
 *
 * WBP_Result 만들기:
 * - CommonUI 버튼 2개를 이름 RestartButton, QuitButton으로 배치한다
 * - Viewmodels 패널에 VX_VM_Result(Manual)을 추가하고 TitleText·StatsText·BuildText 등을 바인딩한다
 * WBP가 없으면 C++ 기본 위젯 트리를 만든다.
 */
UCLASS()
class VOXELCASTER_API UVXResultWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetResult(const FVXRunResult& Result, const UVXModifierComponent* Modifiers);

protected:
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> QuitButton;

private:
	void BuildDefaultTree();
	void HandleRestart();
	void HandleQuit();

	UPROPERTY(Transient)
	TObjectPtr<UVX_VM_Result> ViewModel;

	bool bBuiltInCode = false;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatsText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BuildText;
};
