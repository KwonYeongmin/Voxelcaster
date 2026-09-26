// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VXDataReimporter.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class ITableRow;
class STableViewBase;

/**
 * DataTable 리임포트 패널 (Tools → VX Data Tools).
 * 폴더의 DataTable 목록과 원본 파일을 보여 주고, 전체 또는 한 줄씩 리임포트한다.
 */
class SVXDataReimportPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVXDataReimportPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** 목록을 다시 읽는다 */
	void Refresh();

	/** 전체 리임포트 (툴바 버튼도 이 함수를 쓴다) */
	void ReimportAll();

private:
	TSharedRef<ITableRow> GenerateRow(TSharedPtr<FVXDataTableEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void ReimportOne(TSharedPtr<FVXDataTableEntry> Item);
	FText GetSummaryText() const;

	TArray<TSharedPtr<FVXDataTableEntry>> Entries;
	TSharedPtr<SListView<TSharedPtr<FVXDataTableEntry>>> ListView;
};
