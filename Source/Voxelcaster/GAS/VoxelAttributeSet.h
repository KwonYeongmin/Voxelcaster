// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "VoxelAttributeSet.generated.h"

#define VOXEL_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 플레이어·적이 공유하는 기본 어트리뷰트.
 * 피해는 Health를 직접 깎지 않고 IncomingDamage(메타)로 들어와 PostGameplayEffectExecute에서 처리한다.
 */
UCLASS()
class VOXELCASTER_API UVoxelAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	VOXEL_ATTRIBUTE_ACCESSORS(UVoxelAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	VOXEL_ATTRIBUTE_ACCESSORS(UVoxelAttributeSet, MaxHealth)

	/** 이동 속도 (cm/s). 6 m/s = 600 */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	VOXEL_ATTRIBUTE_ACCESSORS(UVoxelAttributeSet, MoveSpeed)

	/** 메타 어트리뷰트: 받은 피해. 저장되지 않고 실행 시 Health로 반영된다. */
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	VOXEL_ATTRIBUTE_ACCESSORS(UVoxelAttributeSet, IncomingDamage)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
