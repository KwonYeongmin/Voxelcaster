// Copyright Epic Games, Inc. All Rights Reserved.

#include "Wave/VXWaveManager.h"
#include "Character/VXCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Enemy/VXEnemyBase.h"
#include "Enemy/VXElite.h"
#include "Enemy/VXRunner.h"
#include "Enemy/VXShooter.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GAS/VXGameplayEffects.h"
#include "Kismet/GameplayStatics.h"
#include "Modifier/VXModifierComponent.h"
#include "Modifier/VXUpgradeSubsystem.h"
#include "Voxelcaster.h"

namespace
{
	FVXWaveSpawn MakeSpawn(TSubclassOf<AVXEnemyBase> EnemyClass, float StartTime, float Interval, int32 CountPerSpawn, int32 TotalCount)
	{
		FVXWaveSpawn Spawn;
		Spawn.EnemyClass = EnemyClass;
		Spawn.StartTime = StartTime;
		Spawn.Interval = Interval;
		Spawn.CountPerSpawn = CountPerSpawn;
		Spawn.TotalCount = TotalCount;
		return Spawn;
	}

	FVXWaveDef MakeWave(std::initializer_list<FVXWaveSpawn> Spawns)
	{
		FVXWaveDef Wave;
		Wave.Spawns = Spawns;
		return Wave;
	}
}

UVXWaveManager::UVXWaveManager()
{
	PrimaryComponentTick.bCanEverTick = true;

	WaveTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Voxelcaster/Data/DT_Waves.DT_Waves")));

	// 테이블을 읽지 못했을 때의 기본값. 적 수는 design 문서(waves.md) 기준
	// MakeSpawn(적, 시작 시각, 주기, 한 번에, 총 수)
	// const TSubclassOf<AVXEnemyBase> Runner = AVXRunner::StaticClass();
	// const TSubclassOf<AVXEnemyBase> Shooter = AVXShooter::StaticClass();
	// const TSubclassOf<AVXEnemyBase> Elite = AVXElite::StaticClass();
	// Waves = {
	// 	MakeWave({ MakeSpawn(Runner, 0.f, 4.f, 2, 12) }),
	// 	MakeWave({ MakeSpawn(Runner, 0.f, 4.f, 2, 16), MakeSpawn(Shooter, 10.f, 10.f, 1, 4) }),
	// 	MakeWave({ MakeSpawn(Runner, 0.f, 4.f, 2, 20), MakeSpawn(Shooter, 8.f, 8.f, 1, 8) }),
	// 	MakeWave({ MakeSpawn(Runner, 0.f, 3.f, 2, 28), MakeSpawn(Shooter, 6.f, 6.f, 1, 12) }),
	// 	MakeWave({ MakeSpawn(Runner, 0.f, 3.f, 2, 30), MakeSpawn(Shooter, 6.f, 8.f, 1, 10), MakeSpawn(Elite, 45.f, 0.f, 1, 1) }),
	// };
}

void UVXWaveManager::BeginPlay()
{
	Super::BeginPlay();

	LoadWavesFromTable();

	if (UVXUpgradeSubsystem* Upgrades = GetWorld()->GetSubsystem<UVXUpgradeSubsystem>())
	{
		Upgrades->OnChoiceApplied.AddUObject(this, &UVXWaveManager::HandleChoiceApplied);
	}

	if (bAutoStart)
	{
		AutoStartTimer = AutoStartDelay;
	}
}

bool UVXWaveManager::LoadWavesFromTable()
{
	if (WaveTable.IsNull())
	{
		UE_LOG(LogVX, Log, TEXT("WaveTable not set: using default waves in code"));
		return false;
	}

	const UDataTable* Table = WaveTable.LoadSynchronous();
	if (nullptr == Table)
	{
		UE_LOG(LogVX, Warning, TEXT("WaveTable '%s' could not be loaded: using default waves in code"), *WaveTable.ToString());
		return false;
	}

	if (Table->GetRowStruct() != FVXWaveRow::StaticStruct())
	{
		UE_LOG(LogVX, Error, TEXT("WaveTable '%s' row struct must be VXWaveRow: using default waves in code"), *Table->GetName());
		return false;
	}

	TArray<FVXWaveRow*> Rows;
	Table->GetAllRows<FVXWaveRow>(TEXT("VXWaveManager"), Rows);
	Rows.RemoveAll([](const FVXWaveRow* Row) { return nullptr == Row; });
	if (Rows.IsEmpty())
	{
		UE_LOG(LogVX, Warning, TEXT("WaveTable '%s' has no rows: using default waves in code"), *Table->GetName());
		return false;
	}

	Rows.Sort([](const FVXWaveRow& A, const FVXWaveRow& B) { return A.Index < B.Index; });

	Waves.Reset();
	for (const FVXWaveRow* Row : Rows)
	{
		FVXWaveDef Wave;
		Wave.Spawns = Row->Spawns;
		Waves.Add(Wave);
	}

	UE_LOG(LogVX, Log, TEXT("Loaded %d waves from '%s'"), Waves.Num(), *Table->GetName());
	return true;
}

// ---------------------------------------------------------------------------
// 진행
// ---------------------------------------------------------------------------

void UVXWaveManager::StartWave(int32 WaveIndex)
{
	if (false == Waves.IsValidIndex(WaveIndex - 1))
	{
		UE_LOG(LogVX, Warning, TEXT("StartWave: invalid wave index %d (total %d)"), WaveIndex, Waves.Num());
		return;
	}

	CurrentWave = WaveIndex;
	State = EVXWaveState::Intro;
	StateTime = 0.f;
	CombatTime = 0.f;
	AutoStartTimer = -1.f;

	BuildSpawnQueue(Waves[WaveIndex - 1]);

	UE_LOG(LogVX, Log, TEXT("Wave %d start: %d enemies to spawn"), CurrentWave, SpawnQueue.Num());
	OnWaveStarted.Broadcast(CurrentWave);
}

void UVXWaveManager::StartNextWave()
{
	StartWave(CurrentWave + 1);
}

void UVXWaveManager::StopWaves()
{
	State = EVXWaveState::Idle;
	AutoStartTimer = -1.f;
	SpawnQueue.Reset();
	SpawnTimes.Reset();
	NextSpawnIndex = 0;
}

int32 UVXWaveManager::GetRemainingEnemies() const
{
	int32 Alive = 0;
	for (const TWeakObjectPtr<AVXEnemyBase>& Enemy : AliveEnemies)
	{
		if (Enemy.IsValid() && false == Enemy->IsDead())
		{
			++Alive;
		}
	}
	return Alive + FMath::Max(0, SpawnQueue.Num() - NextSpawnIndex);
}

void UVXWaveManager::BuildSpawnQueue(const FVXWaveDef& Wave)
{
	SpawnQueue.Reset();
	SpawnTimes.Reset();
	NextSpawnIndex = 0;

	// 스폰 규칙을 (시각, 적 클래스) 목록으로 펼친 뒤 시각순으로 정렬한다.
	TArray<TPair<float, TSubclassOf<AVXEnemyBase>>> Entries;
	for (const FVXWaveSpawn& Spawn : Wave.Spawns)
	{
		if (nullptr == Spawn.EnemyClass.Get())
		{
			UE_LOG(LogVX, Warning, TEXT("Wave %d: spawn rule without EnemyClass, skipping %d"), CurrentWave, Spawn.TotalCount);
			continue;
		}

		const int32 PerSpawn = FMath::Max(1, Spawn.CountPerSpawn);
		for (int32 Spawned = 0; Spawned < Spawn.TotalCount; ++Spawned)
		{
			const int32 Batch = Spawned / PerSpawn;
			Entries.Emplace(Spawn.StartTime + Spawn.Interval * Batch, Spawn.EnemyClass);
		}
	}

	Entries.StableSort([](const TPair<float, TSubclassOf<AVXEnemyBase>>& A, const TPair<float, TSubclassOf<AVXEnemyBase>>& B)
	{
		return A.Key < B.Key;
	});

	for (const TPair<float, TSubclassOf<AVXEnemyBase>>& Entry : Entries)
	{
		SpawnTimes.Add(Entry.Key);
		SpawnQueue.Add(Entry.Value);
	}
}

// ---------------------------------------------------------------------------
// 틱
// ---------------------------------------------------------------------------

void UVXWaveManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BindPlayerIfNeeded();
	CompactAliveList();

	if (CurrentWave > 0 && EVXWaveState::Finished != State && EVXWaveState::Idle != State)
	{
		PlayTime += DeltaTime;
	}

	if (AutoStartTimer >= 0.f)
	{
		AutoStartTimer -= DeltaTime;
		if (AutoStartTimer < 0.f && State == EVXWaveState::Idle)
		{
			StartWave(1);
		}
	}

	switch (State)
	{
	case EVXWaveState::Intro:
		StateTime += DeltaTime;
		if (StateTime >= IntroDuration)
		{
			BeginCombat();
		}
		break;

	case EVXWaveState::Combat:
		CombatTime += DeltaTime;
		UpdateSpawning();
		UpdateClearCondition();
		break;

	case EVXWaveState::Cleared:
		if (bAutoAdvance && false == bWaitingForReward)
		{
			StateTime += DeltaTime;
			if (StateTime >= AutoAdvanceDelay)
			{
				StartNextWave();
			}
		}
		break;

	default:
		break;
	}

	if (bShowDebugInfo)
	{
		ShowDebugInfo();
	}
}

void UVXWaveManager::BeginCombat()
{
	State = EVXWaveState::Combat;
	CombatTime = 0.f;
}

void UVXWaveManager::UpdateSpawning()
{
	while (NextSpawnIndex < SpawnQueue.Num() && CombatTime >= SpawnTimes[NextSpawnIndex])
	{
		if (AliveEnemies.Num() >= MaxConcurrentEnemies)
		{
			break; // 빈자리가 날 때까지 대기
		}

		if (false == SpawnEnemy(SpawnQueue[NextSpawnIndex]))
		{
			break; // 위치를 못 찾았으면 다음 틱에 다시 시도
		}
		++NextSpawnIndex;
	}
}

void UVXWaveManager::UpdateClearCondition()
{
	if (NextSpawnIndex < SpawnQueue.Num() || AliveEnemies.Num() > 0)
	{
		return;
	}

	State = EVXWaveState::Cleared;
	StateTime = 0.f;
	UE_LOG(LogVX, Log, TEXT("Wave %d cleared (kills: %d)"), CurrentWave, KillCount);
	OnWaveCleared.Broadcast(CurrentWave);

	if (CurrentWave >= Waves.Num())
	{
		State = EVXWaveState::Finished;
		UE_LOG(LogVX, Log, TEXT("All waves cleared: victory"));
		OnGameWon.Broadcast();
		return;
	}

	// 웨이브 클리어 회복 (마지막 웨이브는 회복 없이 종료)
	AVXCharacterBase* Player = FindLivePlayer();
	if (Player)
	{
		VXEffects::ApplyHeal(Player->GetAbilitySystemComponent(), ClearHealAmount);
	}

	// 보상 카드 3장 중 1장 선택. 고르면 HandleChoiceApplied에서 다음 웨이브를 시작한다.
	bWaitingForReward = false;
	if (bRewardBetweenWaves && Player)
	{
		UVXUpgradeSubsystem* Upgrades = GetWorld()->GetSubsystem<UVXUpgradeSubsystem>();
		UVXModifierComponent* Modifiers = Player->FindComponentByClass<UVXModifierComponent>();
		if (Upgrades && Modifiers)
		{
			bWaitingForReward = Upgrades->DrawChoices(Modifiers);
		}
	}
}

void UVXWaveManager::HandleChoiceApplied(const FVXUpgradeCard& Card)
{
	if (bWaitingForReward && EVXWaveState::Cleared == State)
	{
		bWaitingForReward = false;
		StartNextWave();
	}
}

// ---------------------------------------------------------------------------
// 스폰
// ---------------------------------------------------------------------------

bool UVXWaveManager::SpawnEnemy(TSubclassOf<AVXEnemyBase> EnemyClass)
{
	UWorld* World = GetWorld();
	AVXCharacterBase* Player = FindLivePlayer();
	if (nullptr == World || nullptr == EnemyClass.Get() || nullptr == Player)
	{
		return false;
	}

	const AVXEnemyBase* EnemyCDO = EnemyClass.GetDefaultObject();
	const float HalfHeight = EnemyCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector Location;
	if (false == FindSpawnLocation(Player, HalfHeight, Location))
	{
		return false;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AVXEnemyBase* Enemy = World->SpawnActor<AVXEnemyBase>(EnemyClass, Location, FRotator::ZeroRotator, Params);
	if (nullptr == Enemy)
	{
		return false;
	}

	RegisterEnemy(Enemy);
	return true;
}

void UVXWaveManager::RegisterEnemy(AVXEnemyBase* Enemy)
{
	Enemy->OnDeath.AddUObject(this, &UVXWaveManager::HandleEnemyDeath);
	Enemy->OnMinionSpawned.AddUObject(this, &UVXWaveManager::RegisterEnemy);
	AliveEnemies.Add(Enemy);
}

bool UVXWaveManager::FindSpawnLocation(const AVXCharacterBase* Player, float CapsuleHalfHeight, FVector& OutLocation) const
{
	UWorld* World = GetWorld();
	const FVector PlayerLocation = Player->GetActorLocation();
	const float FeetZ = PlayerLocation.Z - Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// 링 위의 후보 지점을 잡고, 아래로 트레이스해서 플레이어와 같은 높이의 바닥 위인 곳만 쓴다.
	// (아레나 밖이나 벽 위는 바닥이 없거나 높이가 달라 걸러진다)
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VXSpawnTrace), false, Player);
	for (int32 Attempt = 0; Attempt < 16; ++Attempt)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Distance = FMath::FRandRange(SpawnRingMin, SpawnRingMax);
		const FVector Candidate = PlayerLocation + FVector::ForwardVector.RotateAngleAxis(Angle, FVector::UpVector) * Distance;

		const FVector Start(Candidate.X, Candidate.Y, PlayerLocation.Z + 300.f);
		const FVector End(Candidate.X, Candidate.Y, FeetZ - 300.f);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams)
			&& Hit.ImpactNormal.Z > 0.9f
			&& FMath::Abs(Hit.ImpactPoint.Z - FeetZ) <= 60.f)
		{
			OutLocation = FVector(Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z + CapsuleHalfHeight + 2.f);
			return true;
		}
	}
	return false;
}

// ---------------------------------------------------------------------------
// 이벤트와 정리
// ---------------------------------------------------------------------------

AVXCharacterBase* UVXWaveManager::FindLivePlayer() const
{
	AVXCharacterBase* Player = Cast<AVXCharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0));
	return (Player && false == Player->IsDead()) ? Player : nullptr;
}

void UVXWaveManager::BindPlayerIfNeeded()
{
	if (bPlayerBound)
	{
		return;
	}

	if (AVXCharacterBase* Player = Cast<AVXCharacterBase>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		Player->OnDeath.AddUObject(this, &UVXWaveManager::HandlePlayerDeath);
		bPlayerBound = true;
	}
}

void UVXWaveManager::CompactAliveList()
{
	AliveEnemies.RemoveAll([](const TWeakObjectPtr<AVXEnemyBase>& Enemy)
	{
		return false == Enemy.IsValid() || Enemy->IsDead();
	});
}

void UVXWaveManager::HandleEnemyDeath(AVXCharacterBase* Enemy)
{
	++KillCount;
	AliveEnemies.RemoveAll([Enemy](const TWeakObjectPtr<AVXEnemyBase>& Entry)
	{
		return Entry.Get() == Enemy;
	});
}

void UVXWaveManager::HandlePlayerDeath(AVXCharacterBase* Player)
{
	if (State == EVXWaveState::Finished)
	{
		return;
	}

	State = EVXWaveState::Finished;
	UE_LOG(LogVX, Log, TEXT("Player died at wave %d: defeat (kills: %d)"), CurrentWave, KillCount);
	OnGameLost.Broadcast();
}

void UVXWaveManager::ShowDebugInfo() const
{
	if (nullptr == GEngine)
	{
		return;
	}

	static const TCHAR* StateNames[] = { TEXT("Idle"), TEXT("Intro"), TEXT("Combat"), TEXT("Cleared"), TEXT("Finished") };
	const FString Text = FString::Printf(TEXT("WAVE %d / %d  [%s]   Remaining: %d   Kills: %d"),
		CurrentWave, Waves.Num(), StateNames[static_cast<int32>(State)], GetRemainingEnemies(), KillCount);
	GEngine->AddOnScreenDebugMessage(7001, 0.f, FColor::Yellow, Text);
}
