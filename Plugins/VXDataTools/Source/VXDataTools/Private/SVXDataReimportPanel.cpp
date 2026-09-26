// Copyright Epic Games, Inc. All Rights Reserved.

#include "SVXDataReimportPanel.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"

namespace
{
	const FName ColumnAsset(TEXT("Asset"));
	const FName ColumnStruct(TEXT("Struct"));
	const FName ColumnSource(TEXT("Source"));
	const FName ColumnStatus(TEXT("Status"));
	const FName ColumnAction(TEXT("Action"));

	const FLinearColor ColorOk(0.35f, 0.85f, 0.45f);
	const FLinearColor ColorWarn(1.f, 0.75f, 0.2f);
	const FLinearColor ColorError(1.f, 0.35f, 0.3f);
	const FLinearColor ColorMuted(0.6f, 0.6f, 0.6f);

	/** 프로젝트 폴더 기준 상대 경로로 줄여 보여 준다 */
	FString ShortenPath(const FString& FullPath)
	{
		FString Path = FullPath;
		FPaths::MakePathRelativeTo(Path, *FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()));
		return Path;
	}

	/** 목록 한 줄 */
	class SVXDataTableRow : public SMultiColumnTableRow<TSharedPtr<FVXDataTableEntry>>
	{
	public:
		SLATE_BEGIN_ARGS(SVXDataTableRow) {}
			SLATE_ARGUMENT(TSharedPtr<FVXDataTableEntry>, Item)
			SLATE_EVENT(FSimpleDelegate, OnReimport)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
		{
			Item = InArgs._Item;
			OnReimport = InArgs._OnReimport;
			SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(FMargin(0.f, 2.f)), OwnerTable);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			if (ColumnAsset == ColumnName)
			{
				return MakeText(FText::FromString(Item->AssetName), FLinearColor::White);
			}
			if (ColumnStruct == ColumnName)
			{
				return MakeText(FText::FromString(Item->RowStructName), ColorMuted);
			}
			if (ColumnSource == ColumnName)
			{
				return SNew(STextBlock)
					.Margin(FMargin(6.f, 0.f))
					.Text_Lambda([this]() { return GetSourceText(); })
					.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetSourceColor()); })
					.ToolTipText_Lambda([this]() { return FText::FromString(Item->SourceFile); });
			}
			if (ColumnStatus == ColumnName)
			{
				return SNew(STextBlock)
					.Margin(FMargin(6.f, 0.f))
					.Text_Lambda([this]() { return FText::FromString(Item->Status); })
					.ColorAndOpacity_Lambda([this]() { return FSlateColor(GetStatusColor()); });
			}
			if (ColumnAction == ColumnName)
			{
				return SNew(SButton)
					.Text(FText::FromString(TEXT("리임포트")))
					.HAlign(HAlign_Center)
					.IsEnabled_Lambda([this]() { return Item->bSourceExists; })
					.OnClicked_Lambda([this]()
					{
						OnReimport.ExecuteIfBound();
						return FReply::Handled();
					});
			}
			return SNullWidget::NullWidget;
		}

	private:
		static TSharedRef<SWidget> MakeText(const FText& Text, const FLinearColor& Color)
		{
			return SNew(STextBlock).Margin(FMargin(6.f, 0.f)).Text(Text).ColorAndOpacity(FSlateColor(Color));
		}

		FText GetSourceText() const
		{
			if (false == Item->bSourceExists)
			{
				return FText::FromString(Item->SourceFile.IsEmpty()
					? FString(TEXT("원본 없음 (Data/에 같은 이름의 csv·json을 두세요)"))
					: TEXT("원본 없음: ") + ShortenPath(Item->SourceFile));
			}
			const FString Path = ShortenPath(Item->SourceFile);
			return FText::FromString(Item->bFallbackSource ? Path + TEXT("  (이름으로 찾음)") : Path);
		}

		FLinearColor GetSourceColor() const
		{
			if (false == Item->bSourceExists)
			{
				return ColorError;
			}
			return Item->bFallbackSource ? ColorWarn : FLinearColor::White;
		}

		FLinearColor GetStatusColor() const
		{
			if (Item->Status.StartsWith(TEXT("완료")))
			{
				return ColorOk;
			}
			return Item->Status.IsEmpty() ? ColorMuted : ColorError;
		}

		TSharedPtr<FVXDataTableEntry> Item;
		FSimpleDelegate OnReimport;
	};
}

void SVXDataReimportPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		.Padding(8.f)
		[
			SNew(SVerticalBox)

			// ---- 폴더
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 6.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("폴더")))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(8.f, 0.f)
				[
					SNew(SEditableTextBox)
					.Text_Lambda([]() { return FText::FromString(VXDataReimporter::GetFolder()); })
					.ToolTipText(FText::FromString(TEXT("이 콘텐츠 폴더(하위 포함)의 DataTable을 대상으로 한다. 예: /Game/Voxelcaster/Data")))
					.OnTextCommitted_Lambda([this](const FText& Text, ETextCommit::Type)
					{
						VXDataReimporter::SetFolder(Text.ToString().TrimStartAndEnd());
						Refresh();
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(FText::FromString(TEXT("새로고침")))
					.OnClicked_Lambda([this]()
					{
						Refresh();
						return FReply::Handled();
					})
				]
			]

			// ---- 전체 리임포트
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), TEXT("PrimaryButton"))
					.ContentPadding(FMargin(16.f, 4.f))
					.Text(FText::FromString(TEXT("전체 리임포트")))
					.ToolTipText(FText::FromString(TEXT("목록의 모든 DataTable을 원본 CSV/JSON에서 다시 읽는다")))
					.IsEnabled_Lambda([this]() { return Entries.Num() > 0; })
					.OnClicked_Lambda([this]()
					{
						ReimportAll();
						return FReply::Handled();
					})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(12.f, 0.f)
				[
					SNew(SCheckBox)
					.IsChecked_Lambda([]() { return VXDataReimporter::GetSaveAfterReimport() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([](ECheckBoxState State) { VXDataReimporter::SetSaveAfterReimport(ECheckBoxState::Checked == State); })
					[
						SNew(STextBlock).Text(FText::FromString(TEXT("리임포트 후 저장")))
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SVXDataReimportPanel::GetSummaryText)
					.ColorAndOpacity(FSlateColor(ColorMuted))
				]
			]

			// ---- 목록
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SAssignNew(ListView, SListView<TSharedPtr<FVXDataTableEntry>>)
				.ListItemsSource(&Entries)
				.SelectionMode(ESelectionMode::None)
				.OnGenerateRow(this, &SVXDataReimportPanel::GenerateRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ColumnAsset).DefaultLabel(FText::FromString(TEXT("에셋"))).FillWidth(0.2f)
					+ SHeaderRow::Column(ColumnStruct).DefaultLabel(FText::FromString(TEXT("행 구조체"))).FillWidth(0.15f)
					+ SHeaderRow::Column(ColumnSource).DefaultLabel(FText::FromString(TEXT("원본 파일"))).FillWidth(0.4f)
					+ SHeaderRow::Column(ColumnStatus).DefaultLabel(FText::FromString(TEXT("결과"))).FillWidth(0.25f)
					+ SHeaderRow::Column(ColumnAction).DefaultLabel(FText::GetEmpty()).FixedWidth(90.f)
				)
			]
		]
	];

	Refresh();
}

void SVXDataReimportPanel::Refresh()
{
	Entries = VXDataReimporter::GatherEntries();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

void SVXDataReimportPanel::ReimportAll()
{
	Refresh();
	VXDataReimporter::ReimportAll(Entries);
	ListView->RequestListRefresh();
}

void SVXDataReimportPanel::ReimportOne(TSharedPtr<FVXDataTableEntry> Item)
{
	if (Item.IsValid())
	{
		VXDataReimporter::Reimport(*Item);
	}
}

TSharedRef<ITableRow> SVXDataReimportPanel::GenerateRow(TSharedPtr<FVXDataTableEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SVXDataTableRow, OwnerTable)
		.Item(Item)
		.OnReimport(FSimpleDelegate::CreateSP(this, &SVXDataReimportPanel::ReimportOne, Item));
}

FText SVXDataReimportPanel::GetSummaryText() const
{
	int32 Missing = 0;
	for (const TSharedPtr<FVXDataTableEntry>& Entry : Entries)
	{
		Missing += Entry->bSourceExists ? 0 : 1;
	}
	return FText::FromString(0 == Missing
		? FString::Printf(TEXT("DataTable %d개"), Entries.Num())
		: FString::Printf(TEXT("DataTable %d개 · 원본 없음 %d개"), Entries.Num(), Missing));
}
