#include "ShowActionSystemEditor.h"
#include "EDataTable.h"
#include "DataTableManager.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
#include "ShowActionMakerGameMode.h"
#include "SSkillDataDetailsWidget.h"

DEFINE_LOG_CATEGORY(ShowActionSystemEditor);

#define LOCTEXT_NAMESPACE "FShowActionSystemEditor"

void FShowActionSystemEditor::StartupModule()
{
    SkillDataTabId = FTabId(TEXT("SkillData Details"));
    SkillShowDataTabId = FTabId(TEXT("SkillShowData Details"));    

    DataTableManager::Get().InitializeDataTables({ 
        EDataTable::SkillData,
        EDataTable::SkillShowData,
        EDataTable::EffectData,
        });

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        SkillDataTabId.TabType,
        FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& TabArgs) -> TSharedRef<SDockTab>
            {
                UDataTable* LoadedSkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        // 데이터 전달
                        SNew(SSkillDataDetailsWidget)
                            .SkillDataTable(LoadedSkillDataTable)
                            .OnSelectAction([this](FName SelectedActionName, FSkillData* SkillData)
                                {
                                    if (this->ShowActionMakerGameMode)
                                    {
                                        this->ShowActionMakerGameMode->SelectAction(SelectedActionName, SkillData);
                                    }                                    
                                })
                    ];

                // 탭이 닫힐 때 데이터 해제
                NewTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([](TSharedRef<SDockTab> ClosedTab)
                    {
                        DataTableManager::Get().ReleaseDatas({ EDataTable::SkillData });
                    }));

                return NewTab;
            }))
        .SetDisplayName(NSLOCTEXT("ShowActionSystem", "SkillDataDetailsTab", "Skill Data Details"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

	UE_LOG(ShowActionSystemEditor, Warning, TEXT("ShowActionSystemEditor module has been loaded"));
}

void FShowActionSystemEditor::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SkillDataTabId.TabType);

    DataTableManager::Get().ReleaseDatas({
        EDataTable::SkillData,
        EDataTable::SkillShowData,
        EDataTable::EffectData,
        });

	UE_LOG(ShowActionSystemEditor, Warning, TEXT("ShowActionSystemEditor module has been unloaded"));
}

void FShowActionSystemEditor::RegisterMenus()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");

    FToolMenuSection& Section = Menu->AddSection("ActionMakerSection", LOCTEXT("ActionMakerSection", "Action Maker"));
    
    Section.AddMenuEntry("SkillDataDetails",
        LOCTEXT("SkillDataDetailsEntry", "SkillData Details"),
        LOCTEXT("SkillDataDetailsEntryTooltip", "Opens the SkillData Details"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FShowActionSystemEditor::OpenSkillDataDetails)));

    Section.AddMenuEntry("SkillShowDataDetails",
        LOCTEXT("SkillShowDataDetailsEntry", "SkillShowData Details"),
        LOCTEXT("SkillShowDataDetailsEntryTooltip", "Opens the SkillShowData Details"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FShowActionSystemEditor::OpenSkillShowDataDetails)));
    
    Section.AddMenuEntry("ShowKeyDataDetails",
        LOCTEXT("ShowKeyDetailsEntry", "ShowKey Details"),
        LOCTEXT("ShowKeyDetailsEntryTooltip", "Opens the ShowKey Details"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FShowActionSystemEditor::OpenShowKeyDetails)));
}

void FShowActionSystemEditor::UnRegisterMenus()
{
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");

    Menu->RemoveSection("ActionMakerSection");
}

void FShowActionSystemEditor::OpenSkillDataDetails()
{
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(SkillDataTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(SkillDataTabId);
    }
}

void FShowActionSystemEditor::OpenSkillShowDataDetails()
{
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(SkillShowDataTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(SkillShowDataTabId);
    }
}

void FShowActionSystemEditor::OpenShowKeyDetails()
{
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(SkillDataTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(SkillDataTabId);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowActionSystemEditor, ShowActionSystemEditor)