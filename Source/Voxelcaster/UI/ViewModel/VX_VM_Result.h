// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VX_VM_Result.generated.h"

/**
 * 결과 화면 (DES-UI-MENU-001). MVVM 뷰모델: WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVX_VM_Result : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InValue);
	bool GetbVictory() const { return bVictory; }
	void SetbVictory(bool InValue);
	const FText& GetStatsText() const { return StatsText; }
	void SetStatsText(const FText& InValue);
	const FText& GetBuildText() const { return BuildText; }
	void SetBuildText(const FText& InValue);
	const FText& GetRestartText() const { return RestartText; }
	void SetRestartText(const FText& InValue);
	const FText& GetQuitText() const { return QuitText; }
	void SetQuitText(const FText& InValue);

private:
	/** 승리 / 패배 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	/** 승리 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	bool bVictory = false;

	/** 도달 웨이브, 처치 수, 플레이 시간 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText StatsText;

	/** 최종 빌드 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText BuildText;

	/** 재시작 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText RestartText;

	/** 종료 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText QuitText;

};
