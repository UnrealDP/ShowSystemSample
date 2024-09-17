// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowSequencerEditorToolkit.h"
#include "RunTime/ShowSequencer.h"

void FShowSequencerEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    // 기본 Details 창 탭 등록
    InTabManager->RegisterTabSpawner("DetailsTab", FOnSpawnTab::CreateSP(this, &FShowSequencerEditorToolkit::SpawnDetailsTab))
        .SetDisplayName(FText::FromString("Details"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());

    // ShowMaker 탭 등록
    InTabManager->RegisterTabSpawner("ShowMakerTab", FOnSpawnTab::CreateSP(this, &FShowSequencerEditorToolkit::SpawnShowMakerTab))
        .SetDisplayName(FText::FromString("ShowMaker"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FShowSequencerEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    // 등록된 탭 스포너 해제
    InTabManager->UnregisterTabSpawner("DetailsTab");
    InTabManager->UnregisterTabSpawner("ShowMakerTab");
}

void FShowSequencerEditorToolkit::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, const TArray<UObject*>& ObjectsToEdit)
{

    // 기본 에디터 레이아웃 설정
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_ShowSequencerEditor_Layout")
        ->AddArea(
            FTabManager::NewPrimaryArea()
            ->Split
			(
				FTabManager::NewStack()
				->AddTab("DetailsTab", ETabState::OpenedTab)
			)
            ->Split
            (
                FTabManager::NewStack()
                ->AddTab("ShowMakerTab", ETabState::OpenedTab)
            )
        );

    

    // 에디터 초기화
    InitAssetEditor(
        Mode,
        InitToolkitHost,
        FName(TEXT("ShowSequencerEditor")),
        Layout,
        true, // 기본 메뉴 생성
        true, // 기본 툴바 생성
        ObjectsToEdit
    );

    GenerateExtendMenuBar();  // 메뉴바 생성

    RegenerateMenusAndToolbars();
}

// 기본 Details 탭 생성
TSharedRef<SDockTab> FShowSequencerEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
    // SDetailsView 생성: 디테일 창에서 기본적으로 프로퍼티를 편집할 수 있도록 설정
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;  // 선택 팁 숨김
    TSharedRef<IDetailsView> DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);

    // 편집할 객체 설정 (디테일 창에 표시할 UObject)
    DetailsView->SetObject(GetEditingObject());

    return SNew(SDockTab)
        .TabRole(ETabRole::PanelTab)
        [
            DetailsView  // 프로퍼티 편집 가능한 디테일 뷰 제공
        ];
}

// ShowMaker 탭 생성
TSharedRef<SDockTab> FShowSequencerEditorToolkit::SpawnShowMakerTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::PanelTab)
        [
            SNew(STextBlock)
                .Text(FText::FromString("ShowMaker Content"))
        ];
}

// 메뉴바 생성
void FShowSequencerEditorToolkit::GenerateExtendMenuBar()
{
    //AddToolbarExtender(GetSharedToolBarExtensibilityManager()->GetAllExtenders(ToolkitCommands, ObjectPtrDecay(EditingObjects)));

   // UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("MainFrame.MainMenu.Window");

    // FExtender 생성
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

    // 지역 함수 정의
    struct Local
    {
        static void ExtendMenu(FMenuBuilder& MenuBuilder, TSharedPtr<FTabManager> InTabManager)
        {
            MenuBuilder.BeginSection("ShowSequencerSettings", FText::FromString("Show Sequencer Settings"));
            {
                // 디테일 탭을 열 수 있는 액션 추가
                const FUIAction OpenDetailsTabAction
                (
                    FExecuteAction::CreateLambda([InTabManager]()
                        {
                            // 탭을 다시 열기 위한 로직
                            if (!InTabManager->FindExistingLiveTab(FName("DetailsTab")).IsValid())
                            {
                                InTabManager->TryInvokeTab(FName("DetailsTab"));
                            }
                        })
                );

                // 메뉴 항목 추가
                MenuBuilder.AddMenuEntry(
                    FText::FromString("Open Details Tab"),
                    FText::FromString("Reopen the Details tab for editing."),
                    FSlateIcon(),
                    OpenDetailsTabAction
                );
            }
            MenuBuilder.EndSection();
        }
    };

    // 메뉴 확장에 지역 함수 연결
    MenuExtender->AddMenuExtension(
        "WindowLayout",  // 원하는 메뉴 위치
        EExtensionHook::After, // 해당 위치 이후에 삽입
        GetToolkitCommands(),
        FMenuExtensionDelegate::CreateStatic(&Local::ExtendMenu, TabManager)
    );

    // 메뉴 확장기 등록
    AddMenuExtender(MenuExtender);
}

