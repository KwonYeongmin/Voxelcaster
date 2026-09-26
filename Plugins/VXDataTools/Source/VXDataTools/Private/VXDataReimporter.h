// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVXDataTools, Log, All);

/** 패널 한 줄: DataTable 하나와 원본 파일 */
struct FVXDataTableEntry
{
	FSoftObjectPath AssetPath;
	FString AssetName;
	FString RowStructName;

	/** 원본 파일 전체 경로. 없으면 빈 문자열 */
	FString SourceFile;
	bool bSourceExists = false;

	/** 임포트 기록의 경로가 없어서 프로젝트 Data/ 폴더에서 에셋 이름으로 찾았는지 */
	bool bFallbackSource = false;

	/** 마지막 리임포트 결과 */
	FString Status;
};

/**
 * DataTable 일괄 리임포트.
 * - 대상: 지정한 콘텐츠 폴더(하위 포함)의 모든 DataTable
 * - 원본: 에셋의 임포트 기록(AssetImportData). 파일이 없으면 <프로젝트>/Data/<에셋 이름>.csv 또는 .json
 * - 설정(폴더, 저장 여부)은 EditorPerProjectUserSettings.ini의 [VXDataTools]에 남는다.
 */
namespace VXDataReimporter
{
	FString GetFolder();
	void SetFolder(const FString& Folder);

	bool GetSaveAfterReimport();
	void SetSaveAfterReimport(bool bSave);

	/** 폴더의 DataTable 목록 (이름 순) */
	TArray<TSharedPtr<FVXDataTableEntry>> GatherEntries();

	/** 하나를 리임포트한다. 결과는 Entry.Status에 남는다 */
	bool Reimport(FVXDataTableEntry& Entry);

	/** 전부 리임포트하고 결과를 알림으로 띄운다 */
	void ReimportAll(TArray<TSharedPtr<FVXDataTableEntry>>& Entries);
}
