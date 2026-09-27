// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"

class AVXEnemyBase;

/**
 * 적 종류 이름(Runner, Shooter, Elite)으로 소환할 클래스를 고른다.
 * - BP(/Game/Voxelcaster/Enemy/BP_VX<종류>)가 있으면 BP: 메시·몽타주 등 에셋 설정이 BP에만 있다
 * - 없으면 C++ 클래스 (기본 도형)
 * 치트 소환과 엘리트 소환이 쓴다. 웨이브는 DT_Waves에 지정한 클래스를 그대로 쓴다.
 */
namespace VXEnemyClasses
{
	/** 모르는 이름이면 nullptr */
	VOXELCASTER_API TSubclassOf<AVXEnemyBase> Resolve(const FString& EnemyType);
}
