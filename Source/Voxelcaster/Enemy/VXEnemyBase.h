// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VXCharacterBase.h"
#include "VXEnemyBase.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UStateTree;
class UAnimMontage;
class UWidgetComponent;
class USkeletalMesh;
class UAnimInstance;
class UBlendSpace;
class UAnimSequenceBase;
struct FVXEnemyRow;
class AVXEnemyBase;

/** 적이 다른 적을 소환했을 때 (웨이브 매니저가 클리어 조건에 포함시킨다) */
DECLARE_MULTICAST_DELEGATE_OneParam(FVXMinionSpawnedSignature, AVXEnemyBase* /*Minion*/);

/**
 * 적 공통 베이스 적 진영, 임시 큐브 메시, 플레이어 탐색, 사망 처리를 제공한다.
 * 이동을 위해 AI 컨트롤러가 자동으로 빙의한다. (컨트롤러가 없으면 캐릭터 이동이 동작하지 않는다)
 */
UCLASS(Abstract)
class VOXELCASTER_API AVXEnemyBase : public AVXCharacterBase
{
	GENERATED_BODY()

public:
	AVXEnemyBase();

	FVXMinionSpawnedSignature OnMinionSpawned;

	/** 치트 EasyMode: 켜면 모든 적의 공격력이 이 값만큼 줄어든다 (0 미만으로는 내려가지 않는다) */
	static constexpr float EasyModeDamageReduction = 5.f;

	/** EasyMode 켜기/끄기. 이미 나와 있는 적과 이후에 나오는 적 모두 공격할 때 적용된다 */
	static void SetEasyMode(bool bEnable) { bEasyMode = bEnable; }
	static bool IsEasyMode() { return bEasyMode; }

	/** 이 적의 행동을 구동하는 StateTree 에셋. 비어 있으면 적 클래스의 기본 행동을 쓴다. */
	const TSoftObjectPtr<UStateTree>& GetStateTreeAsset() const { return StateTreeAsset; }

	/** AI 컨트롤러가 StateTree를 실행 중인지 */
	bool IsDrivenByStateTree() const;

	virtual void Tick(float DeltaSeconds) override;

	/** 디버그: 지금 따라가는 경로 지점 수 (0이면 직진) */
	int32 GetPathPointCount() const { return PathPoints.Num(); }

protected:
	virtual void PostInitializeComponents() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// ---- 캐릭터 메시 ----
	// 우선순위: DT_Enemies의 SkeletalMesh > EnemySkeletalMesh(C++ 기본값·BP) > 메시 컴포넌트에 직접 넣은 메시 > 임시 큐브

	/** 스켈레탈 메시. 비어 있으면 메시 컴포넌트에 직접 지정한 메시를 그대로 쓴다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	TSoftObjectPtr<USkeletalMesh> EnemySkeletalMesh;

	/** 애니메이션 블루프린트. 비어 있으면 메시 컴포넌트의 설정을 그대로 쓴다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	TSoftClassPtr<UAnimInstance> EnemyAnimClass;

	/**
	 * 애님 BP 없이 쓰는 이동 애니메이션. EnemyAnimClass가 비어 있을 때만 쓴다.
	 * 블렌드스페이스의 가로축을 (현재 속도 / 최대 속도)로 움직인다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

	/** 멈춰 있을 때 재생할 애니메이션 (선택) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	TSoftObjectPtr<UAnimSequenceBase> IdleAnimation;

	/** 이 속도(cm/s) 아래면 멈춘 것으로 본다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	float IdleSpeedThreshold = 20.f;

	/** 메시 위치 보정 (cm). 기본은 발이 캡슐 바닥에 오도록 캡슐 절반 높이만큼 내린 위치 기준 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	FVector MeshLocationOffset = FVector::ZeroVector;

	/** 메시 회전 (Yaw). 언리얼 기본 캐릭터 메시는 Y축을 보고 있어 -90으로 돌린다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	float MeshYaw = -90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Mesh")
	float MeshScale = 1.f;

	/**
	 * Target에게 가려면 지금 어느 방향으로 움직여야 하는지 (수평 단위 벡터).
	 * - 사이에 벽이 없으면 직진
	 * - 벽에 가려 있으면 내비메시 경로의 다음 지점으로 (레벨에 Nav Mesh Bounds Volume이 있어야 한다. 없으면 직진)
	 */
	FVector GetPathDirectionTo(const AActor* Target);

	/** 경로를 다시 계산하는 간격 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Enemy")
	float PathRefreshInterval = 0.3f;

	/** EnemySkeletalMesh·EnemyAnimClass를 메시 컴포넌트에 적용한다 (지정되어 있을 때만) */
	void ApplySkeletalMesh();

	/** 블렌드스페이스 이동 애니메이션을 시작한다 (애님 BP가 없을 때) */
	void StartBlendSpaceLocomotion();
	void TickBlendSpaceLocomotion();
	virtual void HandleDamaged(float Amount) override;

	/** 처치 히트스톱 (실제 시간 초). 엘리트는 더 길다 (DES-FEEL-001) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float KillHitStop = 0.05f;

	/** 처치 시 게임패드 진동 (0이면 없음). 엘리트만 준다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float KillVibrationIntensity = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Feel")
	float KillVibrationDuration = 0.3f;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleDeath() override;

	// ---- 군집 이동 (Boids: 분리·정렬·결합). 매 틱 이동 입력에 더해진다. 가중치는 DT_Enemies에서 덮어쓴다 ----

	/** 분리 반경 (cm). 이 거리 안의 다른 적(종류 무관)에게서 멀어진다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float SeparationRadius = 140.f;

	/** 분리 힘 (0이면 끔) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float SeparationWeight = 0.8f;

	/** 이웃 거리 (cm). 이 안의 같은 종류 적과 정렬·결합한다 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float FlockRadius = 400.f;

	/** 정렬 힘: 이웃의 평균 진행 방향 (0이면 끔) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float AlignmentWeight = 0.f;

	/** 결합 힘: 이웃의 평균 위치 (0이면 끔) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float CohesionWeight = 0.f;

	/**
	 * 플레이어와 이 거리(cm) 안이면 정렬·결합을 끈다. 두 배 거리부터 서서히 약해진다.
	 * 가까이 와서는 무리를 풀고 플레이어를 둘러싸게 한다. (분리는 계속 동작)
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Flock")
	float FlockReleaseDistance = 350.f;

	/**
	 * DT_Enemies의 행을 능력치에 적용한다. 자식 클래스는 공격 관련 값을 덧붙여 적용한다.
	 * 체력·이동 속도는 어트리뷰트 초기화 전에 적용된다.
	 */
	virtual void ApplyEnemyStats(const FVXEnemyRow& Row);

	/** DT_Enemies에서 찾을 행 이름 (Runner, Shooter, Elite) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Enemy")
	FName StatRowName;

	/** 공격할 때 실제로 주는 피해 (EasyMode 반영) */
	float GetAdjustedAttackDamage(float BaseDamage) const;

	/** 살아 있는 플레이어 캐릭터. 없으면 nullptr */
	AVXCharacterBase* FindLivePlayer() const;

	/** 몸 색상을 바꾼다 (예고 연출 등). 임시 큐브 메시를 쓸 때만 동작한다 */
	void SetBodyColor(const FLinearColor& Color);

	/** 블루프린트에서 스켈레탈 메시를 지정했는지 (임시 큐브 대신 실제 캐릭터를 쓰는지) */
	bool HasSkeletalMesh() const;

	/** 임시 메시 색상 (복셀 에셋 확정 전) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FLinearColor BodyColor = FLinearColor(0.8f, 0.1f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	FVector BodyScale = FVector(0.6f, 0.6f, 1.6f);

	/** StateTree 에셋 경로 (자식 클래스 생성자에서 지정) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|AI")
	TSoftObjectPtr<UStateTree> StateTreeAsset;

	/**
	 * 사망 애니메이션. 지정하면 사망 시 재생하고, 끝날 때까지 시체를 남긴다.
	 * 블루프린트에서 스켈레탈 메시(Mesh)를 지정하면 임시 큐브(BodyMesh)는 자동으로 숨겨진다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Visual")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** 사망 후 제거까지 시간 (초) */
	UPROPERTY(EditDefaultsOnly, Category = "Voxel|Enemy")
	float DeathLifeSpan = 0.2f;

private:
	/** 임시 큐브. 스켈레탈 메시가 없을 때만 보인다 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	/** 머리 위 HP 바 (화면 공간 빌보드) */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	void UpdateHealthBar(float Current, float Max);

	/** 보이는 메시 (스켈레탈 메시가 있으면 그것, 없으면 임시 큐브) */
	USceneComponent* GetVisualMesh() const;

	/** 피격 플래시·펀치가 끝나는 실제 시간. 0이면 진행 중 아님 */
	double FlashEndRealTime = 0.0;
	FVector VisualBaseScale = FVector::OneVector;

	static bool bEasyMode;

	/** 살아 있는 적 목록 (겹침 방지 계산용). 월드가 여러 개(PIE)여도 같은 월드끼리만 계산한다 */
	static TArray<TWeakObjectPtr<AVXEnemyBase>> AliveEnemies;

	/** 블렌드스페이스로 이동 애니메이션을 재생 중인지 */
	bool bBlendSpaceLocomotion = false;
	bool bPlayingIdle = false;

	UPROPERTY(Transient)
	TObjectPtr<UBlendSpace> LoadedBlendSpace;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequenceBase> LoadedIdle;

	/** 군집 조향: 분리 + 정렬 + 결합을 이동 입력에 더한다 */
	void ApplyFlocking();

	/** 길찾기 경로 (월드 좌표). 비어 있으면 직진 */
	TArray<FVector> PathPoints;
	int32 PathIndex = 0;
	float NextPathTime = 0.f;
	float DeathWorldTime = -1.f;
};
