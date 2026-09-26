// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VXHealthBarWidget.generated.h"

class UProgressBar;

/** 적 머리 위 HP 바 (빌보드). 위젯 트리를 코드로 만든다. */
UCLASS()
class VOXELCASTER_API UVXHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetHealth(float Current, float Max);

private:
	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> Bar;
};
