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
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowMaker/ShowSequencerNotifyHook.h"
#include "RunTime/ShowSequencerComponent.h"

DEFINE_LOG_CATEGORY(ShowActionSystemEditor);

#define LOCTEXT_NAMESPACE "FShowActionSystemEditor"

void FShowActionSystemEditor::StartupModule()
{
    SkillDataTabId = FTabId(TEXT("SkillData Details"));
    ShowKeyDetailsTabId = FTabId(TEXT("ShowKey Details"));
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
                UDataTable* LoadedSkillShowDataTable = DataTableManager::DataTable(EDataTable::SkillShowData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        // 데이터 전달
                        SAssignNew(SkillDataDetailsWidget, SSkillDataDetailsWidget)
                            .SkillDataTable(LoadedSkillDataTable)
                            .SkillShowDataTable(LoadedSkillShowDataTable)
                            .OnSelectAction(FSelectActionFunction::CreateRaw(this, &FShowActionSystemEditor::SelectAction))
                            .OnActionPropertyChanged(FPropertyActionDataChanged::CreateRaw(this, &FShowActionSystemEditor::NotifyActionChange))
                            .OnActionShowPropertyChanged(FPropertyActionShowDataChanged::CreateRaw(this, &FShowActionSystemEditor::NotifyActionShowChange))
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

    FDetailsViewArgs ShowKeyDetailsViewArgs;
    ShowKeyDetailsViewArgs.bAllowSearch = true;
    ShowKeyDetailsViewArgs.bShowOptions = true;
    ShowKeyNotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(nullptr));
    ShowKeyDetailsViewArgs.NotifyHook = ShowKeyNotifyHookInstance.Get();

    FStructureDetailsViewArgs ShowKeyDetailsArgs;
    ShowKeyDetailsArgs.bShowObjects = true;

    ShowKeyStructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
        ShowKeyDetailsViewArgs,
        ShowKeyDetailsArgs,
        nullptr,
        FText::FromString("Show Key Details")
    );

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        ShowKeyDetailsTabId.TabType,
        FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& TabArgs) -> TSharedRef<SDockTab>
            {
                UDataTable* LoadedSkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        ShowKeyStructureDetailsView->GetWidget().ToSharedRef()
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
                            .OnAddKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBase)
								{
                                    EditorHelper->EditShowSequencer->MarkPackageDirty();
								})
                            .OnSelectedKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBase)
								{
                                    if (EditorHelper->SelectedShowBase != ShowBase)
                                    {
                                        EditorHelper->SelectedShowBase = ShowBase;
                                        UpdateShowKeyDetails(EditorHelper);
                                    }
								})
                            .OnRemoveKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
                                {
                                    TObjectPtr<UShowBase> CheckSelectedShowBase = EditorHelper->CheckGetSelectedShowBase();
                                    if (CheckSelectedShowBase != EditorHelper->SelectedShowBase)
                                    {
                                        EditorHelper->SelectedShowBase = CheckSelectedShowBase;
                                        UpdateShowKeyDetails(EditorHelper);
                                    }
                                })
                            .OnPlay_Lambda([this]()
								{
									if (ShowActionMakerGameMode)
									{
										ShowActionMakerGameMode->DoAction();
									}
								})
                    ];

                return NewTab;
            }))
        .SetDisplayName(NSLOCTEXT("ShowActionSystem", "ShowActionControllPanelsTab", "Action Controll"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FShowActionSystemEditor::UpdateShowKeyDetails(TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
{
    if (EditorHelper->SelectedShowBase)
    {
        UScriptStruct* ScriptStruct = EditorHelper->GetShowKeyStaticStruct(EditorHelper->SelectedShowBase);
        FShowKey* ShowKeyPtr = EditorHelper->GetMutableShowKey(EditorHelper->SelectedShowBase);
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(ScriptStruct, (uint8*)ShowKeyPtr));
        ShowKeyStructureDetailsView->SetStructureData(StructData);
        ShowKeyNotifyHookInstance->UpdateEditorHelper(EditorHelper.Get());
    }
    else
    {
        ShowKeyStructureDetailsView->SetStructureData(nullptr);
    }
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
    TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(ShowKeyDetailsTabId);

    if (ExistingTab.IsValid())
    {
        // 이미 열려 있는 경우 탭에 포커스를 주거나 원하는 작업 수행
        ExistingTab->DrawAttention();
    }
    else
    {
        // 탭이 열려 있지 않은 경우 새로 열기
        FGlobalTabmanager::Get()->TryInvokeTab(ShowKeyDetailsTabId);
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
            if (Action->ActionBaseShowData)
            {
                if (Action->ActionBaseShowData->CastShow.IsValid())
                {
                    Action->NewShowSequencer(EActionState::Cast);
                }
                if (Action->ActionBaseShowData->ExecShow.IsValid())
                {
                    Action->NewShowSequencer(EActionState::Exec);
                }
                if (Action->ActionBaseShowData->FinishShow.IsValid())
                {
                    Action->NewShowSequencer(EActionState::Finish);
                }
            }

            if (ShowActionControllPanels)
            {
                ShowActionControllPanels->SelectAction(Action, Action->CastShow, Action->ExecShow, Action->FinishShow);
            }
        }
    }
}

void FShowActionSystemEditor::NotifyActionChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillData* SkillData)
{
    FFieldVariant FieldVariant = PropertyChangedEvent.Property->Owner;

    if (UStruct* Struct = PropertyChangedEvent.Property->GetOwnerStruct())
    {
        /*if (SelectedShowBase)
        {
            FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
            FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

            if (SelectedShowBase->IsA<UShowAnimStatic>())
            {
                if (PropertyName.IsEqual("AnimSequenceAsset"))
                {
                    SelectedShowBase->ExecuteReset();
                }
            }
        }*/
    }
}

void FShowActionSystemEditor::NotifyActionShowChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillShowData* SkillShowData)
{
    if (ShowActionMakerGameMode)
    {
        if (UStruct* Struct = PropertyChangedEvent.Property->GetOwnerStruct())
        {
            FName PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
            FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

            if (PropertyName.IsEqual("CastShow"))
            {
                if (SkillShowData)
                {
                    ChangeShow(EActionState::Cast, SkillShowData);

                }
            }
            else if (PropertyName.IsEqual("ExecShow"))
			{
                if (SkillShowData)
                {
                    ChangeShow(EActionState::Exec, SkillShowData);

                }
			}
            else if (PropertyName.IsEqual("FinishShow"))
            {
                if (SkillShowData)
                {
                    ChangeShow(EActionState::Finish, SkillShowData);

                }
            }
        }
    }
}

void FShowActionSystemEditor::ChangeShow(EActionState ActionState, FSkillShowData* SkillShowData)
{
    if (SkillShowData)
    {
        FSoftObjectPath* NewShowPath = nullptr;
        switch (ActionState)
        {
        case EActionState::Cast:
            NewShowPath = &SkillShowData->CastShow;
            break;
        case EActionState::Exec:
            NewShowPath = &SkillShowData->ExecShow;
            break;
        case EActionState::Finish:
            NewShowPath = &SkillShowData->FinishShow;
            break;
        default:
            return;
        }

        TObjectPtr<UShowSequencer> NewShowSequencer = nullptr;
        UActionBase* Action = ShowActionMakerGameMode ? ShowActionMakerGameMode->CrrAction : nullptr;
        if (Action)
        {
            if (NewShowPath->IsValid())
            {
                NewShowSequencer = Action->NewShowSequencer(ActionState);
            }
            else
            {
                AActor* Owner = Action->GetOwner();
                UShowSequencerComponent* ShowSequencerComponent = Owner->FindComponentByClass<UShowSequencerComponent>();
                switch (ActionState)
                {
                case EActionState::Cast:
                    ShowSequencerComponent->DisposeShow(Action->CastShow);
                    Action->CastShow = nullptr;
                    break;
                case EActionState::Exec:
                    ShowSequencerComponent->DisposeShow(Action->ExecShow);
                    Action->ExecShow = nullptr;
                    break;
                case EActionState::Finish:
                    ShowSequencerComponent->DisposeShow(Action->FinishShow);
                    Action->FinishShow = nullptr;
                    break;
                default:
                    break;
                }
            }
        }

        if (ShowActionControllPanels)
        {
            ShowActionControllPanels->ChangeShow(ActionState, NewShowSequencer);
        }
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowActionSystemEditor, ShowActionSystemEditor)