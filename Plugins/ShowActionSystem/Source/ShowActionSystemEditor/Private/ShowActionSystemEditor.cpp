#include "ShowActionSystemEditor.h"
#include "EDataTable.h"
#include "DataTableManager.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
#include "ShowActionMakerGameMode.h"
#include "SSkillDataDetailsWidget.h"
#include "RunTime/ShowBase.h"
#include "RunTime/ShowSystem.h"
#include "SShowActionControllPanels.h"
#include "ActionBase.h"

DEFINE_LOG_CATEGORY(ShowActionSystemEditor);

#define LOCTEXT_NAMESPACE "FShowActionSystemEditor"

void FShowActionSystemEditor::StartupModule()
{
    SkillDataTabId = FTabId(TEXT("SkillData Details"));
    SkillShowKeyDetailsTabId = FTabId(TEXT("ShowKey Details"));
    ShowActionControllPanelsTabId = FTabId(TEXT("Action Controll"));

    DataTableManager::Get().InitializeDataTables({ 
        EDataTable::SkillData,
        EDataTable::SkillShowData,
        EDataTable::EffectData,
        });

    RegisterSkillDataTab();
    RegisterShowKeyDetailsTab();
    RegisterShowActionControllPanelsTab();

	UE_LOG(ShowActionSystemEditor, Warning, TEXT("ShowActionSystemEditor module has been loaded"));
}

void FShowActionSystemEditor::RegisterSkillDataTab()
{
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
                            .OnSelectAction_Lambda([this](FName SelectedActionName, FSkillData* SkillData, FSkillShowData* SkillShowData)
                                {
                                    SelectAction(SelectedActionName, SkillData, SkillShowData);
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
}

void FShowActionSystemEditor::RegisterShowKeyDetailsTab()
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs SkillShowDetailsViewArgs;
    SkillShowDetailsViewArgs.bAllowSearch = true;
    SkillShowDetailsViewArgs.bShowOptions = true;
    //NotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(EditorHelper.Get()));
    //SkillShowDetailsViewArgs.NotifyHook = NotifyHookInstance.Get();

    FStructureDetailsViewArgs SkillShowDetailsArgs;
    SkillShowDetailsArgs.bShowObjects = true;

    StructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
        SkillShowDetailsViewArgs,
        SkillShowDetailsArgs,
        nullptr,
        FText::FromString("Show Key Details")
    );

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        SkillShowKeyDetailsTabId.TabType,
        FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& TabArgs) -> TSharedRef<SDockTab>
            {
                UDataTable* LoadedSkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        StructureDetailsView->GetWidget().ToSharedRef()
                    ];

                return NewTab;
            }))
        .SetDisplayName(NSLOCTEXT("ShowActionSystem", "ShowKeyDetailsTab", "ShowKey Details"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FShowActionSystemEditor::RegisterShowActionControllPanelsTab()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        ShowActionControllPanelsTabId.TabType,
        FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& TabArgs) -> TSharedRef<SDockTab>
            {
                UDataTable* LoadedSkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        SAssignNew(ShowActionControllPanels, SShowActionControllPanels)
                    ];

                return NewTab;
            }))
        .SetDisplayName(NSLOCTEXT("ShowActionSystem", "ShowActionControllPanelsTab", "Action Controll"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FShowActionSystemEditor::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SkillDataTabId.TabType);

    if (DataTableManager::IsInitialized())
    {
        DataTableManager::Get().ReleaseDatas({
            EDataTable::SkillData,
            EDataTable::SkillShowData,
            EDataTable::EffectData,
            });
    }

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

    Section.AddMenuEntry("ShowKeyDataDetails",
        LOCTEXT("ShowKeyDetailsEntry", "ShowKey Details"),
        LOCTEXT("ShowKeyDetailsEntryTooltip", "Opens the ShowKey Details"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FShowActionSystemEditor::OpenShowKeyDetails)));

    Section.AddMenuEntry("ShowActionControllPanels",
        LOCTEXT("ShowKeyDetailsEntry", "Action Controll"),
        LOCTEXT("ShowKeyDetailsEntryTooltip", "Opens the ShowActionControllPanels"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FShowActionSystemEditor::OpenShowActionControllPanels)));
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

void FShowActionSystemEditor::OpenShowKeyDetails()
{
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(SkillShowKeyDetailsTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(SkillShowKeyDetailsTabId);
    }
}

void FShowActionSystemEditor::OpenShowActionControllPanels()
{
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(ShowActionControllPanelsTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(ShowActionControllPanelsTabId);
    }
}

void FShowActionSystemEditor::SelectAction(FName InSelectedActionName, FSkillData* InSkillData, FSkillShowData* InSkillShowData)
{
    if (ShowActionMakerGameMode)
    {
        if (UActionBase* Action = ShowActionMakerGameMode->SelectAction(InSelectedActionName, InSkillData, InSkillShowData))
        {
            TObjectPtr<UShowSequencer> OutCastShow;
            TObjectPtr<UShowSequencer> OutExecShow;
            TObjectPtr<UShowSequencer> OutFinishShow;
            Action->EditorLoadAllShow(OutCastShow, OutExecShow, OutFinishShow);

            if (ShowActionControllPanels)
            {
                ShowActionControllPanels->SelectAction(Action, OutCastShow, OutExecShow, OutFinishShow);
            }
        }
    }
}

void FShowActionSystemEditor::SetShowBase(UShowBase* NewShowBase)
{
    if (SelectedShowBase == NewShowBase)
    {
        return;
    }

    SelectedShowBase = NewShowBase;
    if (SelectedShowBase)
    {
        UScriptStruct* ScriptStruct = ShowSystem::GetShowKeyStaticStruct(SelectedShowBase->GetKeyType());
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(ScriptStruct, (uint8*)SelectedShowBase->GetShowKey()));
        StructureDetailsView->SetStructureData(StructData);
    }
    else
    {
        StructureDetailsView = nullptr;
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowActionSystemEditor, ShowActionSystemEditor)