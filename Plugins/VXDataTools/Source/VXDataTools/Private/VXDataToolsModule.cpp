// Copyright Epic Games, Inc. All Rights Reserved.

#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "SVXDataReimportPanel.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "VXDataReimporter.h"
#include "VXToonPostProcess.h"
#include "Widgets/Docking/SDockTab.h"

/**
 * VX Data Tools 에디터 모듈.
 * - 레벨 에디터 툴바: [데이터 리임포트] 한 번 누르면 폴더의 DataTable 전부 리임포트, 옆 ▼에서 패널 열기
 * - Tools 메뉴: VX Data Tools 패널
 */
class FVXDataToolsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TabName, FOnSpawnTab::CreateRaw(this, &FVXDataToolsModule::SpawnTab))
			.SetDisplayName(FText::FromString(TEXT("VX Data Tools")))
			.SetTooltipText(FText::FromString(TEXT("DataTable을 원본 CSV/JSON에서 한 번에 리임포트")))
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.DataTable")))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVXDataToolsModule::RegisterMenus));
	}

	virtual void ShutdownModule() override
	{
		if (UObjectInitialized())
		{
			UToolMenus::UnRegisterStartupCallback(this);
			UToolMenus::UnregisterOwner(this);
		}
		if (FSlateApplication::IsInitialized())
		{
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
		}
	}

private:
	void RegisterMenus()
	{
		FToolMenuOwnerScoped OwnerScoped(this);

		// ---- 툴바
		if (UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User")))
		{
			FToolMenuSection& Section = Toolbar->FindOrAddSection(TEXT("VXDataTools"));

			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				TEXT("VXReimportAllData"),
				FUIAction(FExecuteAction::CreateRaw(this, &FVXDataToolsModule::ReimportAll)),
				FText::FromString(TEXT("데이터 리임포트")),
				FText::FromString(TEXT("DataTable 전부를 원본 CSV/JSON에서 다시 읽는다 (폴더는 VX Data Tools 패널에서 설정)")),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Refresh"))));

			Section.AddEntry(FToolMenuEntry::InitComboButton(
				TEXT("VXDataToolsMenu"),
				FUIAction(),
				FNewToolMenuDelegate::CreateLambda([this](UToolMenu* Menu)
				{
					FToolMenuSection& MenuSection = Menu->AddSection(TEXT("VXDataTools"));
					AddOpenPanelEntry(MenuSection);
					AddToonPostProcessEntry(MenuSection);
				}),
				FText::FromString(TEXT("데이터 도구")),
				FText::FromString(TEXT("VX Data Tools")),
				FSlateIcon(),
				true));
		}

		// ---- Tools 메뉴
		if (UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools")))
		{
			FToolMenuSection& Section = ToolsMenu->FindOrAddSection(TEXT("VXDataTools"), FText::FromString(TEXT("Voxelcaster")));
			AddOpenPanelEntry(Section);
			AddToonPostProcessEntry(Section);
		}
	}

	void AddOpenPanelEntry(FToolMenuSection& Section)
	{
		Section.AddMenuEntry(
			TEXT("VXOpenDataTools"),
			FText::FromString(TEXT("VX Data Tools")),
			FText::FromString(TEXT("DataTable 목록·원본 파일·리임포트 결과를 보는 패널")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.DataTable")),
			FUIAction(FExecuteAction::CreateLambda([]() { FGlobalTabmanager::Get()->TryInvokeTab(TabName); })));
	}

	void AddToonPostProcessEntry(FToolMenuSection& Section)
	{
		Section.AddMenuEntry(
			TEXT("VXApplyToonPostProcess"),
			FText::FromString(TEXT("카툰 포스트 프로세스 적용")),
			FText::FromString(TEXT("현재 레벨의 Post Process Volume에 MI_PP_Toon, 할레이션, 필름 그레인, 색수차, 비네트를 설정한다 (Ctrl+Z로 되돌리기)")),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.PostProcessVolume")),
			FUIAction(FExecuteAction::CreateLambda([]() { VXToonPostProcess::ApplyToCurrentLevel(); })));
	}

	/** 툴바 버튼: 패널이 열려 있으면 패널 목록도 갱신된다 */
	void ReimportAll()
	{
		if (TSharedPtr<SVXDataReimportPanel> Panel = PanelWeak.Pin())
		{
			Panel->ReimportAll();
			return;
		}

		TArray<TSharedPtr<FVXDataTableEntry>> Entries = VXDataReimporter::GatherEntries();
		VXDataReimporter::ReimportAll(Entries);
	}

	TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& Args)
	{
		TSharedRef<SVXDataReimportPanel> Panel = SNew(SVXDataReimportPanel);
		PanelWeak = Panel;
		return SNew(SDockTab).TabRole(ETabRole::NomadTab)[Panel];
	}

	static const FName TabName;
	TWeakPtr<SVXDataReimportPanel> PanelWeak;
};

const FName FVXDataToolsModule::TabName(TEXT("VXDataTools"));

IMPLEMENT_MODULE(FVXDataToolsModule, VXDataTools)
