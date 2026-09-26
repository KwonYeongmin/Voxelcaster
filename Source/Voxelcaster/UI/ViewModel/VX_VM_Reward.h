// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "UI/ViewModel/VX_VM_RewardCard.h"
#include "VX_VM_Reward.generated.h"

/**
 * 보상 선택 화면 (DES-UI-REWARD-001). MVVM 뷰모델: WBP의 View Bindings로 위젯에 연결한다.
 * 값은 C++(플레이어 컨트롤러·화면 베이스 클래스)이 넣고, 바뀐 값만 알린다.
 */
UCLASS(BlueprintType, meta = (MVVMAllowedContextCreationType = "Manual"))
class VOXELCASTER_API UVX_VM_Reward : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UVX_VM_Reward();

	const FText& GetTitleText() const { return TitleText; }
	void SetTitleText(const FText& InValue);
	const FText& GetHintText() const { return HintText; }
	void SetHintText(const FText& InValue);
	const FText& GetBuildText() const { return BuildText; }
	void SetBuildText(const FText& InValue);
	int32 GetCardCount() const { return CardCount; }
	void SetCardCount(int32 InValue);
	UVX_VM_RewardCard* GetCard0() const { return Card0; }
	UVX_VM_RewardCard* GetCard1() const { return Card1; }
	UVX_VM_RewardCard* GetCard2() const { return Card2; }

private:
	/** 웨이브 2 클리어 - 강화를 선택하세요 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText TitleText;

	/** 조작 안내 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText HintText;

	/** 포커스된 카드 스킬의 현재 빌드 + 미리보기 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	FText BuildText;

	/** 카드 수 (1~3) */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	int32 CardCount = 0;

	/** 왼쪽 카드 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVX_VM_RewardCard> Card0;

	/** 가운데 카드 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVX_VM_RewardCard> Card1;

	/** 오른쪽 카드 */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Voxel", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVX_VM_RewardCard> Card2;

};
