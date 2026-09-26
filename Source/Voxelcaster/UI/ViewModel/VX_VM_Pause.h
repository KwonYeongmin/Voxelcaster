// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/ViewModel/VX_VM_Base.h"
#include "VX_VM_Pause.generated.h"

/**
 * 일시정지 화면 (DES-UI-MENU-001). MVVM 뷰모델 (UVX_VM_Base 상속): WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVX_VM_Pause : public UVX_VM_Base
{
	GENERATED_BODY()

public:
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InValue);
	const FText& GetResumeText() const { return ResumeText; }
	void SetResumeText(const FText& InValue);
	const FText& GetRestartText() const { return RestartText; }
	void SetRestartText(const FText& InValue);
	const FText& GetQuitText() const { return QuitText; }
	void SetQuitText(const FText& InValue);

private:
	/** 일시정지 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	/** 재개 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText ResumeText;

	/** 재시작 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText RestartText;

	/** 종료 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText QuitText;

};
