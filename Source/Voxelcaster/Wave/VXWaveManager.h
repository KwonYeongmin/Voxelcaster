// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/VXWaveData.h"
#include "VXWaveManager.generated.h"

class AVXEnemyBase;
class AVXCharacterBase;
class UDataTable;

/** 웨이브 정의 (DES-WAVE-001). 런타임에 DT_Waves(FVXWaveRow)에서 채운다. */
USTRUCT(BlueprintType)
struct FVXWaveDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVXWaveSpawn> Spawns;
};

UENUM(BlueprintType)
enum class EVXWaveState : uint8
{
	Idle,
	/** 웨이브 시작 연출 (스폰 전) */
	Intro,
	/** 스폰 + 전투 */
	Combat,
	/** 클리어 후 다음 웨이브 대기 */
	Cleared,
	/** 승리 또는 패배로 종료 */
	Finished
};

DECLARE_MULTICAST_DELEGATE_OneParam(FVXWaveEventSignature, int32 /*WaveIndex (1부터)*/);
DECLARE_MULTICAST_DELEGATE(FVXGameEndSignature);

/**
 * 웨이브 진행과 적 스폰을 담당한다 (SPC-ENEMY-001). 게임 모드가 소유한다.
 * - 스폰: 웨이브의 스폰 규칙(적 클래스, 시작 시각, 주기, 한 번에 나오는 수, 총 수)대로 내보낸다. 동시 최대 30마리
 * - 위치: 플레이어 기준 12~15m 링, 바닥이 있는 지점
 * - 클리어: 스폰이 모두 끝나고 필드의 적이 0이 되면 클리어. 체력 +30 (마지막 웨이브 제외)
 */
UCLASS(ClassGroup = (Voxel), meta = (BlueprintSpawnableComponent))
class VOXELCASTER_API UVXWaveManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UVXWaveManager();

	/** 웨이브를 시작한다 (1부터). 이미 나온 적은 그대로 둔다. */
	void StartWave(int32 WaveIndex);
	void StartNextWave();

	/**
	 * 테스트 웨이브: 러너·슈터·엘리트를 지정한 수만큼 한 번에 소환한다 (치트 TestWave).
	 * 클리어해도 회복·보상·다음 웨이브·승리 처리를 하지 않고 대기 상태로 돌아간다.
	 */
	void StartTestWave(int32 RunnerCount, int32 ShooterCount, int32 EliteCount);
	bool IsTestWave() const { return bIsTestWave; }

	/** 진행을 멈춘다. 이미 나온 적은 남는다. */
	void StopWaves();

	int32 GetCurrentWave() const { return CurrentWave; }
	int32 GetTotalWaves() const { return Waves.Num(); }
	/** 웨이브 정의 (1부터). 없으면 nullptr */
	const FVXWaveDef* GetWaveDef(int32 WaveIndex) const { return Waves.IsValidIndex(WaveIndex - 1) ? &Waves[WaveIndex - 1] : nullptr; }
	EVXWaveState GetState() const { return State; }
	/** 필드의 적 + 아직 스폰 안 된 적 */
	int32 GetRemainingEnemies() const;
	int32 GetKillCount() const { return KillCount; }
	/** 첫 웨이브 시작부터 종료까지 흐른 시간 (초). 일시정지 중에는 멈춘다 */
	float GetPlayTime() const { return PlayTime; }

	FVXWaveEventSignature OnWaveStarted;
	FVXWaveEventSignature OnWaveCleared;
	FVXGameEndSignature OnGameWon;
	FVXGameEndSignature OnGameLost;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * 웨이브 데이터 테이블 (행 구조체: FVXWaveRow). 수치 조정은 이 테이블에서 한다.
	 * 비어 있거나 읽지 못하면 아래 Waves의 코드 기본값(design 문서의 5웨이브)을 쓴다.
	 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave", meta = (RequiredAssetDataTags = "RowStructure=/Script/Voxelcaster.VXWaveRow"))
	TSoftObjectPtr<UDataTable> WaveTable;

	/** 웨이브 목록. WaveTable을 읽으면 그 내용으로 바뀐다. (테이블이 없을 때의 기본값) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	TArray<FVXWaveDef> Waves;

	/** 게임 시작 시 자동으로 웨이브 1을 시작한다 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	bool bAutoStart = true;

	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float AutoStartDelay = 1.f;

	/** 웨이브 시작 연출 시간 (초) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float IntroDuration = 2.f;

	/** 필드 동시 존재 적 수 상한 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	int32 MaxConcurrentEnemies = 30;

	/** 스폰 링 반경 (cm). 12~15m */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float SpawnRingMin = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float SpawnRingMax = 1500.f;

	/** 웨이브 클리어 시 체력 회복량 */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float ClearHealAmount = 30.f;

	/** 웨이브 사이에 보상 카드를 고르게 한다. 고르면 다음 웨이브가 시작된다. (마지막 웨이브 제외) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	bool bRewardBetweenWaves = true;

	/** 클리어 후 자동으로 다음 웨이브로 넘어간다. (보상을 고르는 중에는 기다린다) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	bool bAutoAdvance = true;

	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	float AutoAdvanceDelay = 3.f;

	/** 화면 좌측 상단에 웨이브 상태를 표시한다 (HUD가 생기면 끈다) */
	UPROPERTY(EditAnywhere, Category = "Voxel|Wave")
	bool bShowDebugInfo = false;

private:
	/** WaveTable에서 웨이브 목록을 읽어 Waves를 채운다. 성공하면 true */
	bool LoadWavesFromTable();

	void BeginCombat();
	void UpdateSpawning();
	void UpdateClearCondition();
	bool SpawnEnemy(TSubclassOf<AVXEnemyBase> EnemyClass);
	/** 적을 추적 목록에 넣고 사망·소환 이벤트를 구독한다. (웨이브 스폰과 소환된 적 공통) */
	void RegisterEnemy(AVXEnemyBase* Enemy);
	bool FindSpawnLocation(const AVXCharacterBase* Player, float CapsuleHalfHeight, FVector& OutLocation) const;
	void BuildSpawnQueue(const FVXWaveDef& Wave);
	AVXCharacterBase* FindLivePlayer() const;
	void BindPlayerIfNeeded();
	void CompactAliveList();
	void ShowDebugInfo() const;

	void HandleChoiceApplied(const struct FVXUpgradeCard& Card);
	void HandleEnemyDeath(AVXCharacterBase* Enemy);
	void HandlePlayerDeath(AVXCharacterBase* Player);

	EVXWaveState State = EVXWaveState::Idle;
	int32 CurrentWave = 0;

	/** 테스트 웨이브 진행 중 (클리어 후 처리 생략) */
	bool bIsTestWave = false;
	int32 KillCount = 0;
	float StateTime = 0.f;
	float CombatTime = 0.f;
	float PlayTime = 0.f;
	float AutoStartTimer = -1.f;
	bool bPlayerBound = false;
	bool bWaitingForReward = false;

	/** 이번 웨이브에서 시간순으로 스폰할 적 클래스 */
	TArray<TSubclassOf<AVXEnemyBase>> SpawnQueue;
	/** SpawnQueue 각 항목이 나와도 되는 전투 경과 시간 */
	TArray<float> SpawnTimes;
	int32 NextSpawnIndex = 0;

	TArray<TWeakObjectPtr<AVXEnemyBase>> AliveEnemies;
};
