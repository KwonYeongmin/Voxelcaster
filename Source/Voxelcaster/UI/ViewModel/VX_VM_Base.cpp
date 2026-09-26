// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/ViewModel/VX_VM_Base.h"

bool UVX_VM_Base::SetTextValue(FText& Value, const FText& NewValue, UE::FieldNotification::FFieldId FieldId)
{
	if (Value.EqualTo(NewValue))
	{
		return false;
	}

	Value = NewValue;
	BroadcastFieldValueChanged(FieldId);
	return true;
}

bool UVX_VM_Base::SetFloatValue(float& Value, float NewValue, UE::FieldNotification::FFieldId FieldId)
{
	if (FMath::IsNearlyEqual(Value, NewValue, FloatTolerance))
	{
		return false;
	}

	Value = NewValue;
	BroadcastFieldValueChanged(FieldId);
	return true;
}
