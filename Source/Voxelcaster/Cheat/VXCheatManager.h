// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "VXCheatManager.generated.h"

/**
 * 개발용 치트 명령 (콘솔 `~`에서 입력). 개발 빌드에서만 동작한다.
 * 플레이어 컨트롤러의 CheatClass로 지정된다.
 */
UCLASS()
class VOXELCASTER_API UVXCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** 무적 토글: 켜면 플레이어 HP가 줄지 않는다. (State.Invincible 태그) */
	virtual void God() override;

	UFUNCTION(Exec)
	void DebugDamage(float Amount = 10.f);

	UFUNCTION(Exec)
	void DebugHeal(float Amount = 30.f);

	UFUNCTION(Exec)
	void DebugKill();

	/** 플레이어 주변 링(8~12m)에 러너를 소환한다. 스킬 테스트용. */
	UFUNCTION(Exec)
	void DebugSpawnRunners(int32 Count = 10);

	/** 지정한 웨이브(1~5)를 시작한다. 인자를 생략하면 1 */
	UFUNCTION(Exec)
	void DebugStartWave(int32 WaveIndex = 1);

	/** 웨이브 진행을 멈춘다. */
	UFUNCTION(Exec)
	void DebugStopWaves();

	/**
	 * 지정한 적을 플레이어 주변(8~12m)에 소환한다. 웨이브와 무관한 테스트용.
	 * 예) DebugSpawnEnemy Elite 1 / DebugSpawnEnemy Shooter 3
	 */
	UFUNCTION(Exec)
	void DebugSpawnEnemy(const FString& EnemyType, int32 Count = 1);

	/** 살아 있는 모든 적에게 피해를 준다. 체력 확인용. 예) DebugDamageEnemies 20 */
	UFUNCTION(Exec)
	void DebugDamageEnemies(float Amount = 20.f);

	/**
	 * 스킬에 모디파이어를 1스택 장착한다. (슬롯·스택 제한 적용)
	 * 예) GiveModifier MagicBolt Explode / GiveModifier Nova Split
	 * 스킬: MagicBolt, Nova, BladeSweep / 모디파이어: Pierce, Split, Explode, Chain, Haste
	 */
	UFUNCTION(Exec)
	void GiveModifier(const FString& Skill, const FString& Modifier);

	/** 보상 선택 화면을 즉시 연다 (웨이브와 무관, 테스트용) */
	UFUNCTION(Exec)
	void ShowUpgradeSelect();

	/** 모든 모디파이어를 뺀다 */
	UFUNCTION(Exec)
	void ClearModifiers();

	/** 살아 있는 모든 적을 제거한다. */
	UFUNCTION(Exec)
	void DebugKillEnemies();

private:
	APawn* GetPlayerPawn() const;
};
