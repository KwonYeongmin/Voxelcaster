// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VX_VM_RewardCard.generated.h"

/**
 * 보상 카드 한 장. MVVM 뷰모델: WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVX_VM_RewardCard : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	const FText& GetSkillName() const { return SkillName; }
	void SetSkillName(const FText& InValue);
	const FText& GetModifierName() const { return ModifierName; }
	void SetModifierName(const FText& InValue);
	const FText& GetLevelText() const { return LevelText; }
	void SetLevelText(const FText& InValue);
	const FText& GetDescription() const { return Description; }
	void SetDescription(const FText& InValue);
	const FText& GetTagText() const { return TagText; }
	void SetTagText(const FText& InValue);
	bool GetbUpgrade() const { return bUpgrade; }
	void SetbUpgrade(bool InValue);
	bool GetbHighlighted() const { return bHighlighted; }
	void SetbHighlighted(bool InValue);

private:
	/** 대상 스킬 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText SkillName;

	/** 모디파이어 이름 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText ModifierName;

	/** 장착 후 스택 (Lv.2) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText LevelText;

	/** 효과 설명 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText Description;

	/** 신규 / 강화 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText TagText;

	/** 이미 장착한 모디파이어를 강화하는 카드인지 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	bool bUpgrade = false;

	/** 포커스·호버 중인지 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	bool bHighlighted = false;

};
