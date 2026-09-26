// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GAS/VXHitContext.h"
#include "VXAbilitySystemComponent.generated.h"

/** 입력 태그(Input.*)로 어빌리티를 활성화하는 기능을 더한 ASC. */
UCLASS()
class VOXELCASTER_API UVXAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** 어빌리티를 부여하고 입력 태그를 스펙에 기록한다. */
	FGameplayAbilitySpecHandle GiveAbilityWithInput(TSubclassOf<UGameplayAbility> AbilityClass, const FGameplayTag& InputTag, int32 Level = 1);

	/**
	 * 스킬 명중의 단일 창구. 대상에게 피해를 주고 OnSkillHit을 발행한다.
	 * 모디파이어(분열·폭발·연쇄 등)는 OnSkillHit에 붙는다. 파생 명중(bIsDerived)은 다른 모디파이어를 다시 발동하지 않는다.
	 */
	void ApplySkillHit(const FVXHitContext& Context);

	/** 스킬이 무언가를 맞혔을 때 (피해 적용 후) */
	FVXSkillHitSignature OnSkillHit;

	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/** 입력이 눌려 있는 어빌리티를 다시 시도한다 (스킬 홀드 자동 재발동). 컨트롤러가 매 프레임 호출한다. */
	void ProcessHeldInputs();

private:
	FGameplayTagContainer HeldInputTags;
};
