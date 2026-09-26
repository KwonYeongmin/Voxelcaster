// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cheat/VXCheatManager.h"
#include "AbilitySystemComponent.h"
#include "Character/VXCharacterBase.h"
#include "Core/VXGameMode.h"
#include "Enemy/VXElite.h"
#include "Enemy/VXRunner.h"
#include "Enemy/VXShooter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GAS/VXGameplayEffects.h"
#include "GAS/VXGameplayTags.h"
#include "Modifier/VXModifierComponent.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "Wave/VXWaveManager.h"
#include "Voxelcaster.h"

APawn* UVXCheatManager::GetPlayerPawn() const
{
	const APlayerController* PC = GetOuterAPlayerController();
	return nullptr != PC ? PC->GetPawn() : nullptr;
}

void UVXCheatManager::God()
{
	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn());
	UAbilitySystemComponent* ASC = nullptr != VoxelChar ? VoxelChar->GetAbilitySystemComponent() : nullptr;
	if (nullptr == ASC)
	{
		return;
	}

	// 피해는 GAS로 들어오므로 엔진 기본 God(bCanBeDamaged) 대신 무적 태그로 막는다.
	const bool bEnable = false == ASC->HasMatchingGameplayTag(VoxelTags::State_God);
	if (bEnable)
	{
		ASC->AddLooseGameplayTag(VoxelTags::State_God);
		ASC->AddLooseGameplayTag(VoxelTags::State_Invincible);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(VoxelTags::State_God);
		ASC->RemoveLooseGameplayTag(VoxelTags::State_Invincible);
	}

	UE_LOG(LogVoxel, Log, TEXT("God mode %s"), bEnable ? TEXT("ON") : TEXT("OFF"));
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		PC->ClientMessage(bEnable ? TEXT("God mode ON") : TEXT("God mode OFF"));
	}
}


void UVXCheatManager::DebugDamage(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 10.f;
	}
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn()))
	{
		VoxelEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), Amount);
	}
}

void UVXCheatManager::DebugHeal(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 30.f;
	}
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn()))
	{
		VoxelEffects::ApplyHeal(VoxelChar->GetAbilitySystemComponent(), Amount);
	}
}

void UVXCheatManager::DebugKill()
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn()))
	{
		VoxelEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), VoxelChar->GetMaxHealth() * 10.f);
	}
}

void UVXCheatManager::DebugSpawnRunners(int32 Count)
{
	// Exec 명령은 C++ 기본 인자를 적용하지 않는다. 인자를 생략하면 0이 들어오므로 기본값으로 보정한다.
	if (Count <= 0)
	{
		Count = 10;
	}

	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn());
	if (nullptr == VoxelChar || nullptr == GetWorld())
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Distance = FMath::FRandRange(800.f, 1200.f);
		const FVector Offset = FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * Distance;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AVXRunner>(AVXRunner::StaticClass(), VoxelChar->GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
	}
}

void UVXCheatManager::DebugSpawnEnemy(const FString& EnemyType, int32 Count)
{
	if (Count <= 0)
	{
		Count = 1;
	}

	TSubclassOf<AVXEnemyBase> EnemyClass;
	if (EnemyType.Equals(TEXT("Runner"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXRunner::StaticClass();
	}
	else if (EnemyType.Equals(TEXT("Shooter"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXShooter::StaticClass();
	}
	else if (EnemyType.Equals(TEXT("Elite"), ESearchCase::IgnoreCase))
	{
		EnemyClass = AVXElite::StaticClass();
	}

	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn());
	if (nullptr == EnemyClass.Get() || nullptr == VoxelChar)
	{
		UE_LOG(LogVoxel, Warning, TEXT("DebugSpawnEnemy: unknown type '%s' (Runner, Shooter, Elite)"), *EnemyType);
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Distance = FMath::FRandRange(800.f, 1200.f);
		const FVector Offset = FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * Distance;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		GetWorld()->SpawnActor<AVXEnemyBase>(EnemyClass, VoxelChar->GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
	}
}

void UVXCheatManager::DebugDamageEnemies(float Amount)
{
	if (Amount <= 0.f)
	{
		Amount = 20.f;
	}

	for (TActorIterator<AVXCharacterBase> It(GetWorld()); It; ++It)
	{
		if (It->GetTeam() == EVXTeam::Enemy && false == It->IsDead())
		{
			VoxelEffects::ApplyDamage(It->GetAbilitySystemComponent(), Amount);
		}
	}
}

void UVXCheatManager::GiveModifier(const FString& Skill, const FString& Modifier)
{
	FGameplayTag SkillTag;
	if (Skill.Equals(TEXT("MagicBolt"), ESearchCase::IgnoreCase))
	{
		SkillTag = VoxelTags::Cooldown_MagicBolt;
	}
	else if (Skill.Equals(TEXT("Nova"), ESearchCase::IgnoreCase))
	{
		SkillTag = VoxelTags::Cooldown_Nova;
	}
	else if (Skill.Equals(TEXT("BladeSweep"), ESearchCase::IgnoreCase))
	{
		SkillTag = VoxelTags::Cooldown_BladeSweep;
	}

	const UEnum* ModifierEnum = StaticEnum<EVXModifierType>();
	const int64 ModifierValue = ModifierEnum->GetValueByNameString(Modifier);

	UVXModifierComponent* Modifiers = nullptr != GetPlayerPawn() ? GetPlayerPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr;
	if (false == SkillTag.IsValid() || INDEX_NONE == ModifierValue || nullptr == Modifiers)
	{
		UE_LOG(LogVoxel, Warning, TEXT("GiveModifier: usage GiveModifier <MagicBolt|Nova|BladeSweep> <Pierce|Split|Explode|Chain|Haste>"));
		return;
	}

	const EVXModifierType Type = static_cast<EVXModifierType>(ModifierValue);
	if (Modifiers->AddModifier(SkillTag, Type))
	{
		UE_LOG(LogVoxel, Log, TEXT("GiveModifier: %s + %s (stack %d)"), *Skill, *Modifier, Modifiers->GetStack(SkillTag, Type));
	}
	else
	{
		UE_LOG(LogVoxel, Warning, TEXT("GiveModifier: cannot add %s to %s (slots full, max stack, or no effect on this skill)"), *Modifier, *Skill);
	}
}

void UVXCheatManager::ShowUpgradeSelect()
{
	UVXUpgradeSubsystem* Upgrades = GetWorld()->GetSubsystem<UVXUpgradeSubsystem>();
	UVXModifierComponent* Modifiers = nullptr != GetPlayerPawn() ? GetPlayerPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr;
	if (Upgrades && Modifiers)
	{
		Upgrades->DrawChoices(Modifiers);
	}
}

void UVXCheatManager::ClearModifiers()
{
	if (UVXModifierComponent* Modifiers = nullptr != GetPlayerPawn() ? GetPlayerPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr)
	{
		Modifiers->ResetModifiers();
	}
}

void UVXCheatManager::DebugKillEnemies()
{
	for (TActorIterator<AVXCharacterBase> It(GetWorld()); It; ++It)
	{
		if (It->GetTeam() == EVXTeam::Enemy && false == It->IsDead())
		{
			VoxelEffects::ApplyDamage(It->GetAbilitySystemComponent(), It->GetMaxHealth() * 10.f);
		}
	}
}

void UVXCheatManager::DebugStartWave(int32 WaveIndex)
{
	if (WaveIndex <= 0)
	{
		WaveIndex = 1;
	}

	if (const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
	{
		GameMode->GetWaveManager()->StartWave(WaveIndex);
	}
}

void UVXCheatManager::DebugStopWaves()
{
	if (const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>())
	{
		GameMode->GetWaveManager()->StopWaves();
	}
}
