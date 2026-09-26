// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "UI/ViewModel/VXSkillSlotViewModel.h"
#include "VXHudViewModel.generated.h"

/**
 * 전투 HUD (DES-UI-HUD-001). MVVM 뷰모델: WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVXHudViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UVXHudViewModel();

	float GetHealthPercent() const { return HealthPercent; }
	void SetHealthPercent(float InValue);
	const FText& GetHealthText() const { return HealthText; }
	void SetHealthText(const FText& InValue);
	bool GetbLowHealth() const { return bLowHealth; }
	void SetbLowHealth(bool InValue);
	const FText& GetWaveText() const { return WaveText; }
	void SetWaveText(const FText& InValue);
	const FText& GetEnemiesText() const { return EnemiesText; }
	void SetEnemiesText(const FText& InValue);
	UVXSkillSlotViewModel* GetSkillSlot0() const { return SkillSlot0; }
	UVXSkillSlotViewModel* GetSkillSlot1() const { return SkillSlot1; }
	UVXSkillSlotViewModel* GetSkillSlot2() const { return SkillSlot2; }
	UVXSkillSlotViewModel* GetSkillSlot3() const { return SkillSlot3; }

private:
	/** 체력 비율 0~1 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	float HealthPercent = 0.f;

	/** HP 70 / 100 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText HealthText;

	/** 체력 30% 이하 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	bool bLowHealth = false;

	/** 웨이브 2 / 5 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText WaveText;

	/** 남은 적 · 처치 수 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText EnemiesText;

	/** 매직 볼트 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVXSkillSlotViewModel> SkillSlot0;

	/** 노바 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVXSkillSlotViewModel> SkillSlot1;

	/** 블레이드 스윕 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVXSkillSlotViewModel> SkillSlot2;

	/** 대시 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVXSkillSlotViewModel> SkillSlot3;

};
