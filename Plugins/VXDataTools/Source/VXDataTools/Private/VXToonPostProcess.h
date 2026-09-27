// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 카툰 포스트 프로세스 설정 (SPC-ART-TOON-001 4절).
 * 현재 레벨의 Post Process Volume에 MI_PP_Toon과 할레이션·필름 그레인·색수차·비네트 값을 넣는다.
 * 노출은 장면 밝기에 따라 맞는 값이 달라서 건드리지 않는다.
 */
namespace VXToonPostProcess
{
	/** 적용한 볼륨 수. 결과는 알림과 로그로도 알린다 */
	int32 ApplyToCurrentLevel();
}
