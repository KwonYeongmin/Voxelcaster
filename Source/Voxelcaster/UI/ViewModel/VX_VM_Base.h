// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VX_VM_Base.generated.h"

/**
 * FText 필드를 바꾸고 내용이 달라졌을 때만 알린다.
 * 매 프레임 같은 문자열로 새 FText를 만들어도 알림이 가지 않는다. (엔진 기본 비교는 IdenticalTo)
 */
#define VX_VM_SET_TEXT(MemberName, NewValue) \
	SetTextValue(MemberName, NewValue, ThisClass::FFieldNotificationClassDescriptor::MemberName)

/** float 필드를 바꾸고 오차 이상 달라졌을 때만 알린다. (쿨다운·체력 비율처럼 매 프레임 조금씩 변하는 값) */
#define VX_VM_SET_FLOAT(MemberName, NewValue) \
	SetFloatValue(MemberName, NewValue, ThisClass::FFieldNotificationClassDescriptor::MemberName)

/**
 * 프로젝트 공통 뷰모델 베이스. 모든 뷰모델(UVX_VM_*)은 이 클래스를 상속한다. (코딩 규약 7.4)
 * - Setter에서 값이 실제로 바뀌었을 때만 알리는 도우미를 제공한다.
 *   FText → VX_VM_SET_TEXT, float → VX_VM_SET_FLOAT, 그 외 → UE_MVVM_SET_PROPERTY_VALUE
 * - WBP의 Viewmodels 패널에서는 이 클래스가 아니라 구체 클래스(VX_VM_Hud 등)를 고른다.
 */
UCLASS(Abstract)
class VOXELCASTER_API UVX_VM_Base : public UMVVMViewModelBase
{
	GENERATED_BODY()

protected:
	/** 알림 판단에 쓰는 float 오차 */
	static constexpr float FloatTolerance = 0.001f;

	bool SetTextValue(FText& Value, const FText& NewValue, UE::FieldNotification::FFieldId FieldId);
	bool SetFloatValue(float& Value, float NewValue, UE::FieldNotification::FFieldId FieldId);
};
