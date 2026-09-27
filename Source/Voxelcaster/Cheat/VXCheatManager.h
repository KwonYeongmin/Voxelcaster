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

	/** 쉬운 모드 토글: 켜면 모든 적의 공격력 -5 (최소 0). 다시 입력하면 끈다 */
	UFUNCTION(Exec)
	void EasyMode();

	/** 레벨 Post Process Volume의 노출 보정(Exposure Compensation)을 게임 중에 바꾼다 (저장되지 않음). 예) DebugExposure 7 */
	UFUNCTION(Exec)
	void DebugExposure(float Bias);

	/** 모든 적이 바라보는 방향(액터·메시)과 플레이어 방향의 차이를 로그에 출력한다 */
	UFUNCTION(Exec)
	void DebugEnemyFacing(float Delay = 0.f);

	/**
	 * Seconds초 동안 플레이어를 앞으로 움직이며 0.5초마다 속도·이동 상태·바닥·프레임을 로그에 남긴다 (이동이 느릴 때 원인 확인용)
	 * 예) DebugMoveTest 3  /  DebugMoveTest 5 10 180 (10초 뒤 5초 동안 -X 방향)
	 */
	UFUNCTION(Exec)
	void DebugMoveTest(float Seconds = 3.f, float Delay = 0.f, float Yaw = 0.f);

	/** 플레이어 폰의 컴포넌트 부착 관계와 위치를 로그에 출력한다 (카메라·메시 위치 확인용) */
	UFUNCTION(Exec)
	void DebugComponents();

	/** 카툰 포스트 프로세스(M_PP_Toon) 파라미터를 모두 로그에 출력한다 */
	UFUNCTION(Exec)
	void ToonParams();

	/** 카툰 포스트 프로세스 파라미터를 게임 중에 바꾼다 (저장되지 않음). 예) ToonParam ShadowFloor 0.15 */
	UFUNCTION(Exec)
	void ToonParam(const FString& Name, float Value);

	/**
	 * Delay초 뒤 스크린샷을 찍는다 (셰이더 컴파일을 기다린 뒤 찍기 위함). bQuit이 1이면 찍고 게임을 끈다.
	 * 예) DebugScreenshot 8 1  →  Saved/Screenshots/ 에 저장
	 */
	UFUNCTION(Exec)
	void DebugScreenshot(float Delay = 5.f, int32 bQuit = 0);

	/** 데이터 테이블을 다시 읽고 검사한다. 프로젝트 세팅에서 테이블 경로를 바꾼 뒤에 쓴다 (이미 부여된 스킬 수치는 재시작 후 반영) */
	UFUNCTION(Exec)
	void DataReload();

	UFUNCTION(Exec)
	void DebugDamage(float Amount = 10.f);

	UFUNCTION(Exec)
	void DebugHeal(float Amount = 30.f);

	UFUNCTION(Exec)
	void DebugKill();

	/** 플레이어 주변 링(8~12m)에 러너를 소환한다. 스킬 테스트용. */
	UFUNCTION(Exec)
	void DebugSpawnRunners(int32 Count = 10);

	/**
	 * 테스트 웨이브: 러너·슈터·엘리트를 한 번에 소환한다. 클리어해도 보상·다음 웨이브 없이 끝난다.
	 * 예) TestWave  → 러너 5, 슈터 2, 엘리트 1 / TestWave 10 3 1 → 러너 10, 슈터 3, 엘리트 1
	 * 0 또는 생략하면 그 종류는 기본값을 쓴다.
	 */
	UFUNCTION(Exec)
	void TestWave(int32 Runners = 5, int32 Shooters = 2, int32 Elites = 1);

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
	/** 레벨 Post Process Volume의 M_PP_Toon 계열 머티리얼을 동적 인스턴스로 바꿔 돌려준다 */
	class UMaterialInstanceDynamic* FindToonMaterial();

	UPROPERTY(Transient)
	TObjectPtr<class UMaterialInstanceDynamic> ToonMID;

	FTimerHandle MoveTestTimer;
	float MoveTestRemaining = 0.f;
	float MoveTestLogTimer = 0.f;
	FVector MoveTestDirection = FVector::ForwardVector;

	APawn* GetPlayerPawn() const;
};
