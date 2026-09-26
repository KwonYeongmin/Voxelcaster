// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "VXPauseWidget.generated.h"

class UCommonButtonBase;
class UVX_VM_Pause;

/**
 * 일시정지 화면 (DES-UI-MENU-001). WBP_Pause의 부모 클래스. CommonUI 활성화 위젯.
 * - 재개 / 재시작 / 종료. 열리면 "재개"에 포커스. Esc · 패드 Menu · B 로 닫고 재개한다
 *
 * WBP_Pause 만들기:
 * - CommonUI 버튼 3개를 이름 ResumeButton, RestartButton, QuitButton으로 배치한다 (WBP_MenuButton 권장)
 * - Viewmodels 패널에 VX_VM_Pause(Manual)을 추가하고 제목·버튼 글자를 바인딩한다
 * WBP가 없으면 C++ 기본 위젯 트리를 만든다.
 */
UCLASS()
class VOXELCASTER_API UVXPauseWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> ResumeButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> RestartButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> QuitButton;

private:
	void BuildDefaultTree();
	void HandleResume();
	void HandleRestart();
	void HandleQuit();

	UPROPERTY(Transient)
	TObjectPtr<UVX_VM_Pause> ViewModel;
};
