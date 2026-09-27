// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/VXUIStyles.h"
#include "Styling/CoreStyle.h"
#include "UI/VXUIBuilder.h"

namespace VXUI
{
	FSlateBrush MakeRoundedBrush(const FLinearColor& Color, float Radius)
	{
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		Brush.TintColor = FSlateColor(Color);
		Brush.OutlineSettings = FSlateBrushOutlineSettings(FVector4(Radius, Radius, Radius, Radius), FSlateColor(FLinearColor::Transparent), 0.f);
		return Brush;
	}
}

// ---------------------------------------------------------------------------
// 텍스트
// ---------------------------------------------------------------------------

void UVXTextStyleBase::Setup(int32 Size, const FLinearColor& InColor, bool bShadow)
{
	// C++ 기본 화면과 같은 기본 폰트 (한글은 엔진 대체 폰트로 그려진다)
	Font = FCoreStyle::GetDefaultFontStyle("Bold", Size);
	Color = InColor;
	bUsesDropShadow = bShadow;
	ShadowOffset = FVector2D(2.f, 2.f);
	ShadowColor = FLinearColor(0.f, 0.f, 0.f, 0.8f);
}

UVXTextStyle_Title::UVXTextStyle_Title()
{
	Setup(30, VXUI::Title, true);
}

UVXTextStyle_Banner::UVXTextStyle_Banner()
{
	Setup(64, VXUI::Title, true);
	ShadowOffset = FVector2D(3.f, 3.f);
}

UVXTextStyle_Body::UVXTextStyle_Body()
{
	Setup(18, VXUI::Body);
}

UVXTextStyle_Muted::UVXTextStyle_Muted()
{
	Setup(14, VXUI::Muted);
}

UVXTextStyle_Accent::UVXTextStyle_Accent()
{
	Setup(16, VXUI::Accent);
}

// ---------------------------------------------------------------------------
// 테두리
// ---------------------------------------------------------------------------

UVXBorderStyle_Panel::UVXBorderStyle_Panel()
{
	Background = VXUI::MakeRoundedBrush(VXUI::Panel);
}

UVXBorderStyle_Dim::UVXBorderStyle_Dim()
{
	Background = VXUI::MakeRoundedBrush(VXUI::Dim, 0.f);
}

// ---------------------------------------------------------------------------
// 버튼
// ---------------------------------------------------------------------------

namespace
{
	void SetupButton(UCommonButtonStyle& Style, const FMargin& Padding)
	{
		Style.bSingleMaterial = false;
		Style.NormalBase = VXUI::MakeRoundedBrush(VXUI::Panel);
		Style.NormalHovered = VXUI::MakeRoundedBrush(VXUI::PanelHighlight);
		Style.NormalPressed = VXUI::MakeRoundedBrush(VXUI::PanelHighlight * FLinearColor(1.3f, 1.3f, 1.3f, 1.f));
		Style.SelectedBase = Style.NormalHovered;
		Style.SelectedHovered = Style.NormalHovered;
		Style.SelectedPressed = Style.NormalPressed;
		Style.Disabled = VXUI::MakeRoundedBrush(FLinearColor(0.1f, 0.1f, 0.1f, 0.6f));
		Style.ButtonPadding = Padding;

		// 포커스·호버 시 글자를 강조색(Title)으로
		Style.NormalTextStyle = UVXTextStyle_Body::StaticClass();
		Style.NormalHoveredTextStyle = UVXTextStyle_Title::StaticClass();
		Style.SelectedTextStyle = UVXTextStyle_Title::StaticClass();
		Style.SelectedHoveredTextStyle = UVXTextStyle_Title::StaticClass();
		Style.DisabledTextStyle = UVXTextStyle_Muted::StaticClass();
	}
}

UVXButtonStyle_Menu::UVXButtonStyle_Menu()
{
	SetupButton(*this, FMargin(24.f, 10.f));
}

UVXButtonStyle_Card::UVXButtonStyle_Card()
{
	SetupButton(*this, FMargin(16.f));
}
