// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cheat/VXCheatManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "Enemy/VXEnemyClasses.h"
#include "Data/VXDataManager.h"
#include "AbilitySystemComponent.h"
#include "Character/VXCharacterBase.h"
#include "Core/VXGameMode.h"
#include "Enemy/VXElite.h"
#include "Enemy/VXEnemyBase.h"
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

void UVXCheatManager::EasyMode()
{
	const bool bEnable = false == AVXEnemyBase::IsEasyMode();
	AVXEnemyBase::SetEasyMode(bEnable);

	const FString Message = FString::Printf(TEXT("Easy mode %s (enemy attack damage -%.0f)"),
		bEnable ? TEXT("ON") : TEXT("OFF"), AVXEnemyBase::EasyModeDamageReduction);
	UE_LOG(LogVX, Log, TEXT("%s"), *Message);
	if (APlayerController* PC = GetOuterAPlayerController())
	{
		PC->ClientMessage(Message);
	}
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
	const bool bEnable = false == ASC->HasMatchingGameplayTag(VXTags::State_God);
	if (bEnable)
	{
		ASC->AddLooseGameplayTag(VXTags::State_God);
		ASC->AddLooseGameplayTag(VXTags::State_Invincible);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(VXTags::State_God);
		ASC->RemoveLooseGameplayTag(VXTags::State_Invincible);
	}

	UE_LOG(LogVX, Log, TEXT("God mode %s"), bEnable ? TEXT("ON") : TEXT("OFF"));
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
		VXEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), Amount);
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
		VXEffects::ApplyHeal(VoxelChar->GetAbilitySystemComponent(), Amount);
	}
}

void UVXCheatManager::DebugKill()
{
	if (const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn()))
	{
		VXEffects::ApplyDamage(VoxelChar->GetAbilitySystemComponent(), VoxelChar->GetMaxHealth() * 10.f);
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
	const TSubclassOf<AVXEnemyBase> RunnerClass = VXEnemyClasses::Resolve(TEXT("Runner"));
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
		GetWorld()->SpawnActor<AVXEnemyBase>(RunnerClass, VoxelChar->GetActorLocation() + Offset, FRotator::ZeroRotator, Params);
	}
}

void UVXCheatManager::DebugSpawnEnemy(const FString& EnemyType, int32 Count)
{
	if (Count <= 0)
	{
		Count = 1;
	}

	// BP가 있으면 BP (메시·몽타주 포함), 없으면 C++ 클래스
	const TSubclassOf<AVXEnemyBase> EnemyClass = VXEnemyClasses::Resolve(EnemyType);

	const AVXCharacterBase* VoxelChar = Cast<AVXCharacterBase>(GetPlayerPawn());
	if (nullptr == EnemyClass.Get() || nullptr == VoxelChar)
	{
		UE_LOG(LogVX, Warning, TEXT("DebugSpawnEnemy: unknown type '%s' (Runner, Shooter, Elite)"), *EnemyType);
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
			VXEffects::ApplyDamage(It->GetAbilitySystemComponent(), Amount);
		}
	}
}

void UVXCheatManager::GiveModifier(const FString& Skill, const FString& Modifier)
{
	FGameplayTag SkillTag;
	if (Skill.Equals(TEXT("MagicBolt"), ESearchCase::IgnoreCase))
	{
		SkillTag = VXTags::Cooldown_MagicBolt;
	}
	else if (Skill.Equals(TEXT("Nova"), ESearchCase::IgnoreCase))
	{
		SkillTag = VXTags::Cooldown_Nova;
	}
	else if (Skill.Equals(TEXT("BladeSweep"), ESearchCase::IgnoreCase))
	{
		SkillTag = VXTags::Cooldown_BladeSweep;
	}

	const UEnum* ModifierEnum = StaticEnum<EVXModifierType>();
	const int64 ModifierValue = ModifierEnum->GetValueByNameString(Modifier);

	UVXModifierComponent* Modifiers = nullptr != GetPlayerPawn() ? GetPlayerPawn()->FindComponentByClass<UVXModifierComponent>() : nullptr;
	if (false == SkillTag.IsValid() || INDEX_NONE == ModifierValue || nullptr == Modifiers)
	{
		UE_LOG(LogVX, Warning, TEXT("GiveModifier: usage GiveModifier <MagicBolt|Nova|BladeSweep> <Pierce|Split|Explode|Chain|Haste>"));
		return;
	}

	const EVXModifierType Type = static_cast<EVXModifierType>(ModifierValue);
	if (Modifiers->AddModifier(SkillTag, Type))
	{
		UE_LOG(LogVX, Log, TEXT("GiveModifier: %s + %s (stack %d)"), *Skill, *Modifier, Modifiers->GetStack(SkillTag, Type));
	}
	else
	{
		UE_LOG(LogVX, Warning, TEXT("GiveModifier: cannot add %s to %s (slots full, max stack, or no effect on this skill)"), *Modifier, *Skill);
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
			VXEffects::ApplyDamage(It->GetAbilitySystemComponent(), It->GetMaxHealth() * 10.f);
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

void UVXCheatManager::DataReload()
{
	if (UVXDataManager* Data = UVXDataManager::Get())
	{
		const bool bOk = Data->LoadAll();
		UE_LOG(LogVX, Log, TEXT("DataReload: %s"), bOk ? TEXT("OK") : TEXT("problems found (see [Data] log)"));
	}
}

void UVXCheatManager::DebugScreenshot(float Delay, int32 bQuit)
{
	if (Delay <= 0.f)
	{
		Delay = 5.f;
	}
	UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return;
	}

	// 게임 시간 기준 (웨이브를 멈추고 쓰면 된다)
	FTimerDelegate Shot = FTimerDelegate::CreateWeakLambda(this, [this, bQuit]()
	{
		FScreenshotRequest::RequestScreenshot(true); // UI 포함
		UE_LOG(LogVX, Log, TEXT("DebugScreenshot: requested"));
		if (bQuit > 0)
		{
			FTimerHandle QuitTimer;
			GetWorld()->GetTimerManager().SetTimer(QuitTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				GetOuterAPlayerController()->ConsoleCommand(TEXT("quit"));
			}), 1.f, false);
		}
	});
	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, Shot, Delay, false);
}

UMaterialInstanceDynamic* UVXCheatManager::FindToonMaterial()
{
	if (ToonMID)
	{
		return ToonMID;
	}
	UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return nullptr;
	}

	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		for (FWeightedBlendable& Blendable : It->Settings.WeightedBlendables.Array)
		{
			UMaterialInterface* Material = Cast<UMaterialInterface>(Blendable.Object);
			if (nullptr == Material || false == Material->GetBaseMaterial()->GetName().Equals(TEXT("M_PP_Toon")))
			{
				continue;
			}
			// 게임 중에만 쓰는 동적 인스턴스로 바꿔 끼운다 (에셋은 바뀌지 않는다)
			ToonMID = Cast<UMaterialInstanceDynamic>(Material);
			if (nullptr == ToonMID)
			{
				ToonMID = UMaterialInstanceDynamic::Create(Material, this);
				Blendable.Object = ToonMID;
			}
			return ToonMID;
		}
	}
	UE_LOG(LogVX, Warning, TEXT("Toon: no Post Process Volume with M_PP_Toon in this level"));
	return nullptr;
}

void UVXCheatManager::ToonParams()
{
	UMaterialInstanceDynamic* Material = FindToonMaterial();
	if (nullptr == Material)
	{
		return;
	}

	TArray<FMaterialParameterInfo> Infos;
	TArray<FGuid> Ids;
	Material->GetAllScalarParameterInfo(Infos, Ids);
	for (const FMaterialParameterInfo& Info : Infos)
	{
		float Value = 0.f;
		Material->GetScalarParameterValue(Info, Value);
		UE_LOG(LogVX, Log, TEXT("Toon scalar %s = %.4f"), *Info.Name.ToString(), Value);
	}

	Infos.Reset();
	Ids.Reset();
	Material->GetAllVectorParameterInfo(Infos, Ids);
	for (const FMaterialParameterInfo& Info : Infos)
	{
		FLinearColor Value;
		Material->GetVectorParameterValue(Info, Value);
		UE_LOG(LogVX, Log, TEXT("Toon vector %s = (%.3f, %.3f, %.3f, %.3f)"), *Info.Name.ToString(), Value.R, Value.G, Value.B, Value.A);
	}
}

void UVXCheatManager::ToonParam(const FString& Name, float Value)
{
	if (UMaterialInstanceDynamic* Material = FindToonMaterial())
	{
		Material->SetScalarParameterValue(*Name, Value);
		UE_LOG(LogVX, Log, TEXT("Toon scalar %s -> %.4f"), *Name, Value);
	}
}

void UVXCheatManager::DebugExposure(float Bias)
{
	int32 Count = 0;
	for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
	{
		It->Settings.bOverride_AutoExposureBias = true;
		It->Settings.AutoExposureBias = Bias;
		++Count;
	}
	UE_LOG(LogVX, Log, TEXT("DebugExposure: bias %.2f on %d volume(s)"), Bias, Count);
}

void UVXCheatManager::DebugComponents()
{
	const APawn* Pawn = GetPlayerController() ? GetPlayerController()->GetPawn() : nullptr;
	if (nullptr == Pawn)
	{
		return;
	}
	UE_LOG(LogVX, Log, TEXT("Components of %s at %s"), *Pawn->GetName(), *Pawn->GetActorLocation().ToString());
	TInlineComponentArray<USceneComponent*> Components(Pawn);
	for (const USceneComponent* Component : Components)
	{
		const USceneComponent* Parent = Component->GetAttachParent();
		UE_LOG(LogVX, Log, TEXT("  %s (%s) parent=%s rel=%s rot=%s scale=%s world=%s"),
			*Component->GetName(), *Component->GetClass()->GetName(), Parent ? *Parent->GetName() : TEXT("-"),
			*Component->GetRelativeLocation().ToString(), *Component->GetRelativeRotation().ToString(),
			*Component->GetRelativeScale3D().ToString(), *Component->GetComponentLocation().ToString());
	}
}

void UVXCheatManager::DebugEnemyFacing(float Delay)
{
	// Delay초 뒤에 찍는다 (소환 직후에는 아직 회전 전이라서)
	if (Delay > 0.f)
	{
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this]() { DebugEnemyFacing(0.f); }), Delay, false);
		return;
	}

	const APawn* Player = GetPlayerController() ? GetPlayerController()->GetPawn() : nullptr;
	if (nullptr == Player)
	{
		return;
	}
	for (TActorIterator<AVXEnemyBase> It(GetWorld()); It; ++It)
	{
		const FVector ToPlayer = (Player->GetActorLocation() - It->GetActorLocation()).GetSafeNormal2D();
		const float WantYaw = ToPlayer.Rotation().Yaw;
		const float ActorYaw = It->GetActorRotation().Yaw;
		USkeletalMeshComponent* SkelMesh = It->GetMesh();
		const bool bHasMesh = SkelMesh && SkelMesh->GetSkeletalMeshAsset();
		UE_LOG(LogVX, Log, TEXT("Facing %s: want %.1f actor %.1f diff %.1f | mesh %s rel yaw %.1f world yaw %.1f | controller %s"),
			*It->GetName(), WantYaw, ActorYaw, FMath::FindDeltaAngleDegrees(ActorYaw, WantYaw),
			bHasMesh ? *SkelMesh->GetSkeletalMeshAsset()->GetName() : TEXT("(cube)"),
			SkelMesh ? SkelMesh->GetRelativeRotation().Yaw : 0.f, SkelMesh ? SkelMesh->GetComponentRotation().Yaw : 0.f,
			It->GetController() ? *It->GetController()->GetName() : TEXT("none"));
		// 캡슐이 벽 등 고정 메시에 파묻혀 있는지 (스폰 위치 검사용)
		const UCapsuleComponent* Capsule = It->GetCapsuleComponent();
		FCollisionQueryParams WallParams(SCENE_QUERY_STAT(VXCheatWall), false, *It);
		const bool bInWall = It->GetWorld()->OverlapAnyTestByObjectType(It->GetActorLocation() + FVector(0.f, 0.f, 10.f), FQuat::Identity,
			FCollisionObjectQueryParams(ECC_WorldStatic),
			FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius() - 2.f, Capsule->GetScaledCapsuleHalfHeight() - 12.f), WallParams);
		UE_LOG(LogVX, Log, TEXT("  dist %.0f | path points %d | in wall %s | loc %s"), FVector::Dist2D(It->GetActorLocation(), Player->GetActorLocation()),
			It->GetPathPointCount(), bInWall ? TEXT("YES") : TEXT("no"), *It->GetActorLocation().ToString());
		if (bHasMesh)
		{
			UE_LOG(LogVX, Log, TEXT("  mesh path %s | anim %s | rel loc %s scale %s | capsule half %.1f radius %.1f"),
				*SkelMesh->GetSkeletalMeshAsset()->GetPathName(),
				SkelMesh->GetAnimClass() ? *SkelMesh->GetAnimClass()->GetPathName() : TEXT("none"),
				*SkelMesh->GetRelativeLocation().ToString(), *SkelMesh->GetRelativeScale3D().ToString(),
				It->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), It->GetCapsuleComponent()->GetUnscaledCapsuleRadius());
		}
	}
}

void UVXCheatManager::TestWave(int32 Runners, int32 Shooters, int32 Elites)
{
	// Exec 명령은 C++ 기본 인자를 적용하지 않는다. 0(생략)이면 기본값으로 보정한다.
	Runners = Runners > 0 ? Runners : 5;
	Shooters = Shooters > 0 ? Shooters : 2;
	Elites = Elites > 0 ? Elites : 1;

	const AVXGameMode* GameMode = GetWorld()->GetAuthGameMode<AVXGameMode>();
	if (nullptr == GameMode)
	{
		UE_LOG(LogVX, Warning, TEXT("TestWave: VXGameMode not found"));
		return;
	}
	GameMode->GetWaveManager()->StartTestWave(Runners, Shooters, Elites);
}

void UVXCheatManager::DebugMoveTest(float Seconds, float Delay, float Yaw)
{
	// 로딩·셰이더 컴파일로 프레임이 낮은 시작 직후를 피하려면 Delay초 뒤에 시작한다
	if (Delay > 0.f)
	{
		FTimerHandle DelayHandle;
		GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateWeakLambda(this, [this, Seconds, Yaw]() { DebugMoveTest(Seconds, 0.f, Yaw); }), Delay, false);
		return;
	}

	MoveTestRemaining = Seconds > 0.f ? Seconds : 3.f;
	MoveTestLogTimer = 0.f;
	MoveTestDirection = FRotator(0.f, Yaw, 0.f).Vector();

	// 매 프레임 앞으로 이동 입력을 넣고, 0.5초마다 상태를 기록한다
	GetWorld()->GetTimerManager().SetTimer(MoveTestTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		ACharacter* Character = Cast<ACharacter>(GetPlayerController() ? GetPlayerController()->GetPawn() : nullptr);
		const float Delta = GetWorld()->GetDeltaSeconds();
		MoveTestRemaining -= Delta;
		MoveTestLogTimer -= Delta;
		if (nullptr == Character || MoveTestRemaining <= 0.f)
		{
			GetWorld()->GetTimerManager().ClearTimer(MoveTestTimer);
			return;
		}

		Character->AddMovementInput(MoveTestDirection, 1.f);
		if (MoveTestLogTimer > 0.f)
		{
			return;
		}
		MoveTestLogTimer = 0.5f;

		const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		const FFindFloorResult& Floor = Movement->CurrentFloor;
		const UPrimitiveComponent* FloorComponent = Floor.HitResult.GetComponent();
		// 발밑 50m까지 무엇이 있는지 (바닥 충돌 확인)
		FHitResult Down;
		FCollisionQueryParams DownParams(SCENE_QUERY_STAT(VXMoveTestDown), false, Character);
		const FVector Start = Character->GetActorLocation();
		const bool bHitDown = GetWorld()->LineTraceSingleByChannel(Down, Start, Start - FVector(0.f, 0.f, 5000.f), ECC_Visibility, DownParams);
		UE_LOG(LogVX, Log, TEXT("MoveTest ground below: %s at %.0f cm"), bHitDown && Down.GetActor() ? *Down.GetActor()->GetName() : TEXT("nothing"), bHitDown ? Down.Distance : -1.f);
		UE_LOG(LogVX, Log, TEXT("MoveTest %s: speed %.0f / max %.0f | mode %s | floor walkable %d dist %.1f on %s | fps %.0f | time dilation world %.2f actor %.2f | loc %s"),
			*Character->GetClass()->GetName(), Character->GetVelocity().Size2D(), Movement->GetMaxSpeed(),
			*Movement->GetMovementName(), Floor.bWalkableFloor ? 1 : 0, Floor.FloorDist,
			FloorComponent ? *FloorComponent->GetOwner()->GetName() : TEXT("none"),
			Delta > 0.f ? 1.f / Delta : 0.f, GetWorld()->GetWorldSettings()->TimeDilation, Character->CustomTimeDilation,
			*Character->GetActorLocation().ToString());
	}), 0.001f, true);
}
