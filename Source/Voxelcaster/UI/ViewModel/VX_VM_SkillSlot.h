// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VX_VM_SkillSlot.generated.h"

/**
 * HUD 스킬 슬롯 하나 (스킬 3개 + 대시). MVVM 뷰모델: WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVX_VM_SkillSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	const FText& GetSkillName() const { return SkillName; }
	void SetSkillName(const FText& InValue);
	const FText& GetKeyText() const { return KeyText; }
	void SetKeyText(const FText& InValue);
	float GetCooldownPercent() const { return CooldownPercent; }
	void SetCooldownPercent(float InValue);
	const FText& GetCooldownText() const { return CooldownText; }
	void SetCooldownText(const FText& InValue);
	bool GetbReady() const { return bReady; }
	void SetbReady(bool InValue);
	const FText& GetModifiersText() const { return ModifiersText; }
	void SetModifiersText(const FText& InValue);

private:
	/** 스킬 이름 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText SkillName;

	/** 입력 키 (장치에 따라 LMB / RT 등) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText KeyText;

	/** 쿨다운 진행도 0~1 (1 = 사용 가능) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	float CooldownPercent = 0.f;

	/** 남은 쿨다운 초. 사용 가능하면 빈 문자열 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText CooldownText;

	/** 사용 가능 여부 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	bool bReady = false;

	/** 장착 모디파이어 ([폭발][연쇄]) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText ModifiersText;

};
