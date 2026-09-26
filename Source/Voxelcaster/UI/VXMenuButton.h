// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "VXMenuButton.generated.h"

class UBorder;
class UTextBlock;

/**
 * 메뉴 버튼 (일시정지·결과 화면). WBP_VX_MenuButton의 부모 클래스. CommonUI 버튼이다.
 * - WBP: 글자 TextBlock을 LabelText, 배경 Border를 ButtonBorder로 이름 지으면 C++이 글자를 넣고 강조 색을 바꾼다.
 * - WBP가 없으면 C++ 기본 위젯 트리를 만든다.
 */
UCLASS()
class VOXELCASTER_API UVXMenuButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	void SetLabel(const FString& Label);

protected:
	/** 버튼 배경 (선택). 포커스·호버 시 색이 바뀐다 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> ButtonBorder;

	/** 버튼 글자 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LabelText;

private:
	void SetHighlighted(bool bInHighlighted);
};
