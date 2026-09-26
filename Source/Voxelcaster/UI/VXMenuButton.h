// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "VXMenuButton.generated.h"

class UBorder;
class UTextBlock;

/** 메뉴 버튼 (일시정지·결과 화면). 위젯 트리를 코드로 만든다. 포커스·호버되면 강조된다. */
UCLASS()
class VOXELCASTER_API UVXMenuButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetLabel(const FString& Label);

private:
	void SetHighlighted(bool bInHighlighted);

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ButtonBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LabelText;
};
