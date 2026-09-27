// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "VXBuildUICommandlet.generated.h"

/**
 * UI 레이아웃 기획서(SPC-UI-LAYOUT-001)대로 WBP_VX_* 의 위젯 트리와 MVVM View Binding을 만든다.
 * - 뷰모델은 WBP에 이미 추가되어 있는 것을 찾아 쓴다 (없으면 Manual로 추가)
 * - 기존 위젯과 바인딩은 지우고 새로 만든다. 실행 전에 Saved/Backup/UI_before_build 에 백업해 둘 것
 * - 에디터를 닫고 실행한다: UnrealEditor-Cmd.exe Voxelcaster.uproject -run=VXBuildUI
 */
UCLASS()
class UVXBuildUICommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UVXBuildUICommandlet();

	virtual int32 Main(const FString& Params) override;
};
