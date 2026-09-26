// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMVVMViewModelBase;
class UTextBlock;
class UUserWidget;
class UWidgetTree;

/**
 * UI 공용 도우미.
 * - 화면은 WBP(디자이너 에셋)를 우선 쓰고, WBP가 없으면 C++로 만든 기본 위젯 트리로 대신한다.
 * - WBP는 MVVM View Bindings로 뷰모델 값을 위젯에 연결한다. (뷰모델 생성 방식: Manual)
 */
namespace VXUI
{
	/** 공용 색 (C++ 기본 위젯 트리용) */
	extern VOXELCASTER_API const FLinearColor Title;
	extern VOXELCASTER_API const FLinearColor Body;
	extern VOXELCASTER_API const FLinearColor Muted;
	extern VOXELCASTER_API const FLinearColor Accent;
	extern VOXELCASTER_API const FLinearColor Panel;
	extern VOXELCASTER_API const FLinearColor PanelHighlight;
	extern VOXELCASTER_API const FLinearColor Dim;

	/** 텍스트 블록을 만든다 (C++ 기본 위젯 트리용) */
	VOXELCASTER_API UTextBlock* MakeText(UWidgetTree* Tree, int32 Size, const FLinearColor& Color, bool bBold = true);

	/**
	 * 위젯의 MVVM 뷰에 뷰모델을 넣는다. WBP의 Viewmodels 패널에 같은 클래스(생성 방식 Manual)가 있어야 연결된다.
	 * C++ 기본 위젯 트리에는 MVVM 뷰가 없으므로 아무 일도 하지 않는다.
	 */
	VOXELCASTER_API void SetViewModel(UUserWidget* Widget, UMVVMViewModelBase* ViewModel);

	/** 텍스트 블록이 있으면 글자를 넣는다 */
	VOXELCASTER_API void SetText(UTextBlock* Text, const FText& Value);
}
