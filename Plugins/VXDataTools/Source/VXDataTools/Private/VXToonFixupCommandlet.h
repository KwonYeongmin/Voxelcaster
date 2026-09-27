// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "VXToonFixupCommandlet.generated.h"

/**
 * M_PP_Toon 셀 셰이딩을 "장면 밝기" 기준에서 "빛의 양" 기준으로 바꾼다 (SPC-ART-TOON-001 3.1).
 * - 휘도 가중치 Constant3Vector에 연결된 Dot의 SceneColor 입력 앞에 SceneColor ÷ Max(BaseColor, 0.01)을 끼운다
 * - Toon Multiply의 Ratio 입력 앞에 Lerp(1, Ratio, HasAlbedo)를 끼운다 (텍스처 색이 0인 하늘·발광면은 원본 유지)
 * - 이미 적용된 머티리얼이면 아무것도 하지 않는다. 에디터를 닫고 실행한다.
 *
 * 실행: UnrealEditor-Cmd.exe Voxelcaster.uproject -run=VXToonFixup
 */
UCLASS()
class UVXToonFixupCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UVXToonFixupCommandlet();

	virtual int32 Main(const FString& Params) override;
};
