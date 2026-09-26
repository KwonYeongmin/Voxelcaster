// Copyright Epic Games, Inc. All Rights Reserved.

#include "VXDataReimporter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "EditorReimportHandler.h"
#include "Engine/DataTable.h"
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Widgets/Notifications/SNotificationList.h"

DEFINE_LOG_CATEGORY(LogVXDataTools);

namespace
{
	const TCHAR* ConfigSection = TEXT("VXDataTools");
	const TCHAR* DefaultFolder = TEXT("/Game/Voxelcaster/Data");

	/** 원본 파일을 찾는다: 임포트 기록 → <프로젝트>/Data/<이름>.csv|.json */
	void ResolveSource(const UDataTable* Table, FVXDataTableEntry& Entry)
	{
		Entry.SourceFile.Reset();
		Entry.bSourceExists = false;
		Entry.bFallbackSource = false;

		const FString Recorded = nullptr != Table->AssetImportData ? Table->AssetImportData->GetFirstFilename() : FString();
		if (false == Recorded.IsEmpty() && FPaths::FileExists(Recorded))
		{
			Entry.SourceFile = Recorded;
			Entry.bSourceExists = true;
			return;
		}

		for (const TCHAR* Extension : { TEXT("csv"), TEXT("json") })
		{
			const FString Candidate = FPaths::ConvertRelativePathToFull(
				FPaths::Combine(FPaths::ProjectDir(), TEXT("Data"), Entry.AssetName + TEXT(".") + Extension));
			if (FPaths::FileExists(Candidate))
			{
				Entry.SourceFile = Candidate;
				Entry.bSourceExists = true;
				Entry.bFallbackSource = true;
				return;
			}
		}

		// 찾지 못했으면 기록된 경로라도 보여 준다
		Entry.SourceFile = Recorded;
	}
}

namespace VXDataReimporter
{
	FString GetFolder()
	{
		FString Folder = DefaultFolder;
		GConfig->GetString(ConfigSection, TEXT("Folder"), Folder, GEditorPerProjectIni);
		return Folder;
	}

	void SetFolder(const FString& Folder)
	{
		GConfig->SetString(ConfigSection, TEXT("Folder"), *Folder, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}

	bool GetSaveAfterReimport()
	{
		bool bSave = true;
		GConfig->GetBool(ConfigSection, TEXT("SaveAfterReimport"), bSave, GEditorPerProjectIni);
		return bSave;
	}

	void SetSaveAfterReimport(bool bSave)
	{
		GConfig->SetBool(ConfigSection, TEXT("SaveAfterReimport"), bSave, GEditorPerProjectIni);
		GConfig->Flush(false, GEditorPerProjectIni);
	}

	TArray<TSharedPtr<FVXDataTableEntry>> GatherEntries()
	{
		IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();

		FARFilter Filter;
		Filter.PackagePaths.Add(*GetFolder());
		Filter.bRecursivePaths = true;
		Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> Assets;
		Registry.GetAssets(Filter, Assets);
		Assets.Sort([](const FAssetData& A, const FAssetData& B) { return A.AssetName.LexicalLess(B.AssetName); });

		TArray<TSharedPtr<FVXDataTableEntry>> Entries;
		for (const FAssetData& Asset : Assets)
		{
			const UDataTable* Table = Cast<UDataTable>(Asset.GetAsset());
			if (nullptr == Table)
			{
				continue;
			}

			TSharedPtr<FVXDataTableEntry> Entry = MakeShared<FVXDataTableEntry>();
			Entry->AssetPath = Asset.GetSoftObjectPath();
			Entry->AssetName = Asset.AssetName.ToString();
			Entry->RowStructName = nullptr != Table->GetRowStruct() ? Table->GetRowStruct()->GetName() : TEXT("None");
			ResolveSource(Table, *Entry);
			Entries.Add(Entry);
		}
		return Entries;
	}

	bool Reimport(FVXDataTableEntry& Entry)
	{
		UDataTable* Table = Cast<UDataTable>(Entry.AssetPath.TryLoad());
		if (nullptr == Table)
		{
			Entry.Status = TEXT("에셋 없음");
			return false;
		}

		ResolveSource(Table, Entry);
		if (false == Entry.bSourceExists)
		{
			Entry.Status = TEXT("원본 파일 없음");
			UE_LOG(LogVXDataTools, Warning, TEXT("%s: source file not found (%s)"), *Entry.AssetName,
				Entry.SourceFile.IsEmpty() ? TEXT("no import record") : *Entry.SourceFile);
			return false;
		}

		// 이름으로 찾은 파일이면 임포트 기록을 그 파일로 바꾼다 (다음부터는 바로 찾는다)
		if (Entry.bFallbackSource)
		{
			FReimportManager::Instance()->UpdateReimportPaths(Table, { Entry.SourceFile });
		}

		// bAutomated = true: 대화상자 없이 진행
		const bool bOk = FReimportManager::Instance()->Reimport(Table, false, false, FString(), nullptr, INDEX_NONE, false, true);
		if (false == bOk)
		{
			Entry.Status = TEXT("실패 (Output Log 확인)");
			UE_LOG(LogVXDataTools, Error, TEXT("%s: reimport failed from %s"), *Entry.AssetName, *Entry.SourceFile);
			return false;
		}

		Entry.bFallbackSource = false;
		Entry.Status = FString::Printf(TEXT("완료 (%d행)"), Table->GetRowMap().Num());
		UE_LOG(LogVXDataTools, Log, TEXT("%s: reimported %d rows from %s"), *Entry.AssetName, Table->GetRowMap().Num(), *Entry.SourceFile);

		if (GetSaveAfterReimport())
		{
			UEditorLoadingAndSavingUtils::SavePackages({ Table->GetPackage() }, true);
		}
		return true;
	}

	void ReimportAll(TArray<TSharedPtr<FVXDataTableEntry>>& Entries)
	{
		int32 Succeeded = 0;
		int32 Failed = 0;
		for (const TSharedPtr<FVXDataTableEntry>& Entry : Entries)
		{
			if (Entry.IsValid())
			{
				Reimport(*Entry) ? ++Succeeded : ++Failed;
			}
		}

		const FString Message = 0 == Failed
			? FString::Printf(TEXT("DataTable %d개 리임포트 완료"), Succeeded)
			: FString::Printf(TEXT("DataTable 리임포트: 성공 %d, 실패 %d (Output Log의 LogVXDataTools 확인)"), Succeeded, Failed);
		UE_LOG(LogVXDataTools, Log, TEXT("Reimport all: %d succeeded, %d failed (%s)"), Succeeded, Failed, *GetFolder());

		FNotificationInfo Info(FText::FromString(Message));
		Info.ExpireDuration = 0 == Failed ? 3.f : 6.f;
		if (TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info))
		{
			Notification->SetCompletionState(0 == Failed ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
		}
	}
}
