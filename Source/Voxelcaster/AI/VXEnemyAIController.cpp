// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/VXEnemyAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Enemy/VXEnemyBase.h"
#include "StateTree.h"
#include "Voxelcaster.h"

AVXEnemyAIController::AVXEnemyAIController()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

	// 컨트롤러는 폰에 빙의하기 전에 BeginPlay를 거친다. 그때는 StateTree 에셋이 없으므로 자동 시작을 끄고
	// OnPossess에서 에셋을 지정한 뒤 직접 시작한다.
	StateTreeComponent->SetStartLogicAutomatically(false);
}

bool AVXEnemyAIController::IsStateTreeRunning() const
{
	return nullptr != StateTreeComponent && StateTreeComponent->IsRunning();
}

void AVXEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const AVXEnemyBase* Enemy = Cast<AVXEnemyBase>(InPawn);
	if (nullptr == Enemy || Enemy->GetStateTreeAsset().IsNull())
	{
		return;
	}

	UStateTree* StateTree = Enemy->GetStateTreeAsset().LoadSynchronous();
	if (nullptr == StateTree)
	{
		UE_LOG(LogVX, Warning, TEXT("%s: StateTree '%s' could not be loaded, falling back to built-in behavior"),
			*GetNameSafe(InPawn), *Enemy->GetStateTreeAsset().ToString());
		return;
	}

	StateTreeComponent->SetStateTree(StateTree);
	StateTreeComponent->StartLogic();
}

void AVXEnemyAIController::OnUnPossess()
{
	if (StateTreeComponent && StateTreeComponent->IsRunning())
	{
		StateTreeComponent->StopLogic(TEXT("Unpossessed"));
	}

	Super::OnUnPossess();
}
