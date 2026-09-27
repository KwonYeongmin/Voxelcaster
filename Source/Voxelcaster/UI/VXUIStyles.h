// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonBorder.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "VXUIStyles.generated.h"

/**
 * 공용 UI 스타일 (SPC-UI-LAYOUT-001 1.2). WBP의 CommonTextBlock·CommonBorder·CommonButton에서 Style로 고른다.
 * 색은 VXUI(C++ 기본 화면)와 같다. 폰트·크기·색은 위젯에서 바꾸지 않고 여기서만 바꾼다.
 */

// ---------------------------------------------------------------------------
// 텍스트
// ---------------------------------------------------------------------------

/** 텍스트 스타일 공통: 기본 폰트(한글 대체 폰트 포함), 굵게 */
UCLASS(Abstract)
class VOXELCASTER_API UVXTextStyleBase : public UCommonTextStyle
{
	GENERATED_BODY()

protected:
	void Setup(int32 Size, const FLinearColor& InColor, bool bShadow = false);
};

/** 화면 제목, 웨이브 표시, 결과 제목 */
UCLASS(meta = (DisplayName = "VX Text Title"))
class VOXELCASTER_API UVXTextStyle_Title : public UVXTextStyleBase
{
	GENERATED_BODY()
public:
	UVXTextStyle_Title();
};

/** 웨이브 시작 배너 (큰 글자, 그림자) */
UCLASS(meta = (DisplayName = "VX Text Banner"))
class VOXELCASTER_API UVXTextStyle_Banner : public UVXTextStyleBase
{
	GENERATED_BODY()
public:
	UVXTextStyle_Banner();
};

/** 체력 글자, 스킬 이름, 카드 설명, 버튼 글자 */
UCLASS(meta = (DisplayName = "VX Text Body"))
class VOXELCASTER_API UVXTextStyle_Body : public UVXTextStyleBase
{
	GENERATED_BODY()
public:
	UVXTextStyle_Body();
};

/** 입력 키 표시, 조작 안내, 보조 설명 */
UCLASS(meta = (DisplayName = "VX Text Muted"))
class VOXELCASTER_API UVXTextStyle_Muted : public UVXTextStyleBase
{
	GENERATED_BODY()
public:
	UVXTextStyle_Muted();
};

/** 쿨다운 초, 모디파이어 목록, 빌드 요약, 배너 아래 줄 */
UCLASS(meta = (DisplayName = "VX Text Accent"))
class VOXELCASTER_API UVXTextStyle_Accent : public UVXTextStyleBase
{
	GENERATED_BODY()
public:
	UVXTextStyle_Accent();
};

// ---------------------------------------------------------------------------
// 테두리
// ---------------------------------------------------------------------------

/** 스킬 슬롯·카드 배경 (둥근 어두운 패널) */
UCLASS(meta = (DisplayName = "VX Border Panel"))
class VOXELCASTER_API UVXBorderStyle_Panel : public UCommonBorderStyle
{
	GENERATED_BODY()
public:
	UVXBorderStyle_Panel();
};

/** 메뉴 화면 뒤 어둡게 (전체 채움) */
UCLASS(meta = (DisplayName = "VX Border Dim"))
class VOXELCASTER_API UVXBorderStyle_Dim : public UCommonBorderStyle
{
	GENERATED_BODY()
public:
	UVXBorderStyle_Dim();
};

// ---------------------------------------------------------------------------
// 버튼 (게임패드 포커스도 Hovered 상태로 그려진다)
// ---------------------------------------------------------------------------

/** 일시정지·결과 버튼 */
UCLASS(meta = (DisplayName = "VX Button Menu"))
class VOXELCASTER_API UVXButtonStyle_Menu : public UCommonButtonStyle
{
	GENERATED_BODY()
public:
	UVXButtonStyle_Menu();
};

/** 보상 카드 */
UCLASS(meta = (DisplayName = "VX Button Card"))
class VOXELCASTER_API UVXButtonStyle_Card : public UCommonButtonStyle
{
	GENERATED_BODY()
public:
	UVXButtonStyle_Card();
};

namespace VXUI
{
	/** 단색 둥근 사각형 브러시 (패널·버튼 배경) */
	VOXELCASTER_API FSlateBrush MakeRoundedBrush(const FLinearColor& Color, float Radius = 6.f);
}
