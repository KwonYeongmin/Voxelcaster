// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "VXEnemyAIController.generated.h"

class UStateTreeAIComponent;

/**
 * 적 AI 컨트롤러. 빙의한 적이 StateTree 에셋을 지정했으면 그 StateTree로 행동을 구동한다. (DES-AI-001)
 * - 에셋이 없거나 불러오지 못하면 StateTree를 실행하지 않는다. 이 경우 적은 자기 Tick의 기본 행동을 쓴다.
 * - 로직은 적 클래스의 공개 함수에 있고, StateTree는 상태와 전환 조건만 조립한다.
 */
UCLASS()
class VOXELCASTER_API AVXEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AVXEnemyAIController();

	/** StateTree가 실제로 실행 중인지 */
	bool IsStateTreeRunning() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
};
