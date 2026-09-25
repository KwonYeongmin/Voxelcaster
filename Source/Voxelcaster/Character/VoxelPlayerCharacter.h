// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/VoxelCharacterBase.h"
#include "VoxelPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UVXModifierComponent;

UCLASS()
class VOXELCASTER_API AVXPlayerCharacter : public AVXCharacterBase
{
	GENERATED_BODY()

public:
	AVXPlayerCharacter();

protected:
	virtual void GrantStartupAbilities() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Voxel|Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	/** 스킬별 모디파이어 장착과 효과 실행 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Modifier")
	TObjectPtr<UVXModifierComponent> ModifierComponent;

	/** 복셀 에셋 확정 전까지 쓰는 임시 큐브 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Voxel|Visual")
	TObjectPtr<UStaticMeshComponent> BodyMesh;
};
