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
#include "RunTime/ShowKeys/ShowCamSequence.h"
#include "ShowMaker/SShowKeyDetailsWidget.h"
#include "ActionServerExecutor.h"


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

void FShowActionSystemEditor::InitializeModule(AShowActionMakerGameMode* InShowActionMakerGameMode)
{
    ShowActionMakerGameMode = InShowActionMakerGameMode;
    ShowActionMakerGameMode->Initialize(FOnUpdateCameraView::CreateRaw(this, &FShowActionSystemEditor::UpdateCameraPathPoint));
    ShowActionMakerGameMode->ChangeTimeScale(CrrTimeScale);

    OpenSkillDataDetails();
    OpenShowKeyDetails();
    OpenShowActionControllPanels();
    RegisterMenus();

    if (SkillDataDetailsWidget)
    {
        FName FirstSkillName = SkillDataDetailsWidget->FirstSkillName();
        if (!FirstSkillName.IsNone())
        {
            SkillDataDetailsWidget->OnSkillSelected(FirstSkillName.ToString());
        }
    }
}

void FShowActionSystemEditor::ClearModule()
{
    UnRegisterMenus();
    ShowActionMakerGameMode = nullptr;

    SelectedShowBasePtr = nullptr;
    CrrActionPtr = nullptr;
    ShowSequencerEditorHelperSortMap.Empty();
    if (ShowActionControllPanels)
    {
        ShowActionControllPanels->RefreshShowActionControllPanels(nullptr, &ShowSequencerEditorHelperSortMap);
    }
    UpdateShowKeyDetails(nullptr, nullptr);
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
    ShowKeyDetailsWidget = SNew(SShowKeyDetailsWidget)
        .InEditorHelper(nullptr)
        .InShowBasePtr(nullptr);

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        ShowKeyDetailsTabId.TabType,
        FOnSpawnTab::CreateLambda([this](const FSpawnTabArgs& TabArgs) -> TSharedRef<SDockTab>
            {
                UDataTable* LoadedSkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

                // 탭 생성
                TSharedRef<SDockTab> NewTab = SNew(SDockTab)
                    .TabRole(ETabRole::NomadTab)
                    [
                        ShowKeyDetailsWidget.ToSharedRef()
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
                            .bIsPlaying_Lambda([this]()
                                {
                                    if (ShowActionMakerGameMode.IsValid() && ShowActionMakerGameMode->CrrActionPtr)
                                    {
                                        if (ShowActionMakerGameMode->CrrActionPtr->GetState() == EActionState::Wait ||
                                            ShowActionMakerGameMode->CrrActionPtr->GetState() == EActionState::Cooldown ||
                                            ShowActionMakerGameMode->CrrActionPtr->GetState() == EActionState::Complete)
                                        {
                                            return false;
                                        }

                                        return !ShowActionMakerGameMode->CrrActionPtr->IsPause();
                                    }
                                    return false;
                                })
                            .Height(20.0f)
                            .OnAddKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBasePtr)
								{
                                    EditorHelper->ShowSequenceAssetMarkPackageDirty();
								})
                            .OnSelectedKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBasePtr)
								{
                                    if (SelectedShowBasePtr != ShowBasePtr)
                                    {
                                        SelectedShowBasePtr = ShowBasePtr;
                                        UpdateShowKeyDetails(EditorHelper, SelectedShowBasePtr);
                                    }
                                    
                                    if (EditorHelper->SelectedShowBasePtr != ShowBasePtr)
                                    {
                                        EditorHelper->SelectedShowBasePtr = ShowBasePtr;
                                    }
								})
                            .OnRemoveKey_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
                                {
                                    UShowBase* CheckSelectedShowBasePtr = EditorHelper->CheckGetSelectedShowBase();
                                    if (CheckSelectedShowBasePtr != EditorHelper->SelectedShowBasePtr)
                                    {
                                        if (SelectedShowBasePtr == EditorHelper->SelectedShowBasePtr)
                                        {
                                            SelectedShowBasePtr = CheckSelectedShowBasePtr;
                                            UpdateShowKeyDetails(EditorHelper, SelectedShowBasePtr);
                                        }

                                        EditorHelper->SelectedShowBasePtr = CheckSelectedShowBasePtr;
                                    }
                                })
                            .OnPlay_Lambda([this]()
								{
									if (ShowActionMakerGameMode.IsValid())
									{
										ShowActionMakerGameMode->DoAction();
									}
								})
                            .IsShowKeySelected_Lambda([this](UShowBase* ShowBasePtr)
                                {
									return SelectedShowBasePtr == ShowBasePtr;
								})
                            .OnTimeScaleValueChanged_Lambda([this](float InValue)
								{
                                    CrrTimeScale = InValue;
									if (ShowActionMakerGameMode.IsValid())
									{
										ShowActionMakerGameMode->ChangeTimeScale(CrrTimeScale);
									}
								})
                    ];

                FSlateApplication::Get().SetKeyboardFocus(ShowActionControllPanels);

                return NewTab;
            }))
        .SetDisplayName(NSLOCTEXT("ShowActionSystem", "ShowActionControllPanelsTab", "Action Controll"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);
    
}

void FShowActionSystemEditor::UpdateShowKeyDetails(TSharedPtr<FShowSequencerEditorHelper> InSelectedShowSequencerEditorHelper, UShowBase* InSelectedShowBasePtr)
{
    if (ShowActionMakerGameMode.IsValid())
    {
        ShowActionMakerGameMode->SelectKey(InSelectedShowSequencerEditorHelper, InSelectedShowBasePtr);
    }

    ShowKeyDetailsWidget->SetShowKey(InSelectedShowSequencerEditorHelper, InSelectedShowBasePtr);
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
    if (ShowActionMakerGameMode.IsValid())
    {
        UShowSequencer* OutCastShowSequencer = nullptr;
        UShowSequencer* OutExecShowSequencer = nullptr;
        UShowSequencer* OutFinishShowSequencer = nullptr;

        if (UActionBase* Action = ShowActionMakerGameMode->SelectAction(
            InSelectedActionName, 
            InSkillData, 
            InSkillShowData,
            OutCastShowSequencer,
            OutExecShowSequencer,
            OutFinishShowSequencer))
        {
            CrrActionPtr = Action;

            for (auto& Elem : ShowSequencerEditorHelperSortMap)
            {
                Elem.Value.Reset();
                Elem.Value = nullptr;
            }
            ShowSequencerEditorHelperSortMap.Empty();

            if (OutCastShowSequencer)
            {
                ShowSequencerEditorHelperSortMap["Cast"] = MakeShared<FShowSequencerEditorHelper>();
                ShowSequencerEditorHelperSortMap["Cast"]->EditShowSequencerPtr = OutCastShowSequencer;
            }
            if (OutExecShowSequencer)
            {
                ShowSequencerEditorHelperSortMap["Exec"] = MakeShared<FShowSequencerEditorHelper>();
                ShowSequencerEditorHelperSortMap["Exec"]->EditShowSequencerPtr = OutExecShowSequencer;
            }
            if (OutFinishShowSequencer)
            {
                ShowSequencerEditorHelperSortMap["Finish"] = MakeShared<FShowSequencerEditorHelper>();
                ShowSequencerEditorHelperSortMap["Finish"]->EditShowSequencerPtr = OutFinishShowSequencer;
            }

            CrrActionPtr->ChangeTimeScale(CrrTimeScale);

            if (ShowActionControllPanels)
            {
                ShowActionControllPanels->RefreshShowActionControllPanels(CrrActionPtr, &ShowSequencerEditorHelperSortMap);
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
    if (ShowActionMakerGameMode.IsValid())
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

        if (ShowActionMakerGameMode.IsValid())
        {
            FString StepStr = StaticEnum<EActionState>()->GetNameStringByValue(static_cast<int64>(ActionState));
            UShowSequencer* NewShowSequencerPtr = ShowActionMakerGameMode->ChangeShow(ActionState, NewShowPath);
            if (NewShowSequencerPtr)
            {
                if (ShowSequencerEditorHelperSortMap.ContainsKey(StepStr))
                {
                    if (TSharedPtr<FShowSequencerEditorHelper>* ExistingHelper = ShowSequencerEditorHelperSortMap.Find(StepStr))
                    {
                        // "Cast" 키가 존재하면 해당 객체의 Show 변수를 변경
                        (*ExistingHelper)->EditShowSequencerPtr = NewShowSequencerPtr;
                    }
                }
                else
                {
                    ShowSequencerEditorHelperSortMap[StepStr] = MakeShared<FShowSequencerEditorHelper>();
                    ShowSequencerEditorHelperSortMap[StepStr]->EditShowSequencerPtr = NewShowSequencerPtr;
                }
            }
            else
            {
                if (ShowSequencerEditorHelperSortMap.ContainsKey(StepStr))
                {
                    ShowSequencerEditorHelperSortMap[StepStr]->Dispose();
                    ShowSequencerEditorHelperSortMap.Remove(StepStr);
                }
            }
        }

        ShowSequencerEditorHelperSortMap.Sort(
            [](const TPair<FString, TSharedPtr<FShowSequencerEditorHelper>>& A, const TPair<FString, TSharedPtr<FShowSequencerEditorHelper>>& B)
            {
                if (A.Key == TEXT("Cast"))
                {
                    return true;
                }
                else if (A.Key == TEXT("Exec") && B.Key != TEXT("Cast"))
                {
                    return true;
                }
                else if (A.Key == TEXT("Finish") && B.Key == TEXT("Finish"))
                {
                    return false;
                }
                return false;
            });


        if (ShowActionControllPanels)
        {
            ShowActionControllPanels->RefreshShowActionControllPanels(CrrActionPtr, &ShowSequencerEditorHelperSortMap);
        }
    }
}

void FShowActionSystemEditor::UpdateCameraPathPoint(APawn* InOwner, FCameraPathPoint* CameraPathPoint, const TArray<AActor*>& ActorsToHide)
{
    UWorld* World = InOwner->GetWorld();
    FVector CasterPos = InOwner->GetActorLocation();

    FVector CameraPos = CameraPathPoint->Position + CasterPos;
    FVector CameraLookAt = CameraPathPoint->LookAtTarget + CasterPos;
    FRotator CameraRotator = (CameraLookAt - CameraPos).Rotation();

    ShowKeyDetailsWidget->UpdateCameraView(World, CameraPos, CameraRotator, ActorsToHide);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowActionSystemEditor, ShowActionSystemEditor)