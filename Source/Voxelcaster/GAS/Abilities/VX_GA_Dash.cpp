// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Abilities/VX_GA_Dash.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/VXCharacterBase.h"
#include "GAS/VXGameplayEffects.h"
#include "GAS/VXGameplayTags.h"
#include "Voxelcaster.h"

UVX_GA_Dash::UVX_GA_Dash()
{
	bBlockedWhileDashing = false;
	CooldownDuration = 1.f;
}

FGameplayTag UVX_GA_Dash::GetCooldownTag() const
{
	return VoxelTags::Cooldown_Dash;
}

void UVX_GA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 쿨다운 시작 (코스트는 없음)
	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogVoxel, Warning, TEXT("Dash: CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AVXCharacterBase* Character = Cast<AVXCharacterBase>(ActorInfo->AvatarActor.Get());
	if (nullptr == Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CurrentHandle = Handle;
	CurrentActorInfo_Cached = ActorInfo;
	CurrentActivationInfo_Cached = ActivationInfo;

	// 대시 상태 + 무적 (대시 시작부터 InvincibleTime 동안)
	FGameplayTagContainer StateTags;
	StateTags.AddTag(VoxelTags::State_Dashing);
	StateTags.AddTag(VoxelTags::State_Invincible);
	if (nullptr == DashStateEffect)
	{
		DashStateEffect = VoxelEffects::MakeTagDurationEffect(this, TEXT("GE_DashState"), StateTags, InvincibleTime);
	}
	ApplyRuntimeEffectToOwner(Handle, ActorInfo, DashStateEffect);

	const FVector Direction = Character->GetDashDirection();
	const float Strength = Distance / FMath::Max(Duration, KINDA_SMALL_NUMBER);

	UAbilityTask_ApplyRootMotionConstantForce* Task = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, Direction, Strength, Duration,
		/*bIsAdditive*/ false, /*StrengthOverTime*/ nullptr,
		ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, /*ClampVelocityOnFinish*/ 0.f,
		/*bEnableGravity*/ true);

	Task->OnFinish.AddDynamic(this, &UVX_GA_Dash::OnDashFinished);
	Task->ReadyForActivation();
}

void UVX_GA_Dash::OnDashFinished()
{
	EndAbility(CurrentHandle, CurrentActorInfo_Cached, CurrentActivationInfo_Cached, true, false);
}
