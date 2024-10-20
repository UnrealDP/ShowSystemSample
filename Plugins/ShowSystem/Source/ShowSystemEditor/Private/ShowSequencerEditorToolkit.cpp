// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowSequencerEditorToolkit.h"
#include "RunTime/ShowSequenceAsset.h"
#include "RunTime/ShowSequencer.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"

bool FShowSequencerEditorToolkit::OnRequestClose() 
{
    UnregisterTabSpawners(GetTabManager().ToSharedRef());

    DetailsView.Reset();
    DetailsView = nullptr;

    EditorHelper->Dispose();
    EditorHelper.Reset();
    EditorHelper = nullptr;

    return true; 
}

void FShowSequencerEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    // 기본 Details 창 탭 등록
    InTabManager->RegisterTabSpawner("DetailsTab", FOnSpawnTab::CreateSP(this, &FShowSequencerEditorToolkit::SpawnDetailsTab))
        .SetDisplayName(FText::FromString("ShowSequence Asset Details"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());

    // ShowMaker 탭 등록
    InTabManager->RegisterTabSpawner("ShowMakerTab", FOnSpawnTab::CreateSP(this, &FShowSequencerEditorToolkit::SpawnShowMakerTab))
        .SetDisplayName(FText::FromString("ShowMaker"))
        .SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FShowSequencerEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    // 등록된 탭 스포너 해제
    InTabManager->UnregisterTabSpawner("DetailsTab");
    InTabManager->UnregisterTabSpawner("ShowMakerTab");

    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
}

// 기본 Details 탭 생성
TSharedRef<SDockTab> FShowSequencerEditorToolkit::SpawnDetailsTab(const FSpawnTabArgs& Args)
{
    // SDetailsView 생성: 디테일 창에서 기본적으로 프로퍼티를 편집할 수 있도록 설정
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bHideSelectionTip = true;  // 선택 팁 숨김
    TSharedRef<IDetailsView> DetailsViewRef = PropertyModule.CreateDetailView(DetailsViewArgs);
    DetailsView = DetailsViewRef;

    // 편집할 객체 설정 (디테일 창에 표시할 UObject)
    UObject* EditingObject = GetEditingObject();
    DetailsView->SetObject(EditingObject);

    return SNew(SDockTab)
        .TabRole(ETabRole::PanelTab)
        [
            DetailsViewRef  // 프로퍼티 편집 가능한 디테일 뷰 제공
        ];
}

// ShowMaker 탭 생성 함수
TSharedRef<SDockTab> FShowSequencerEditorToolkit::SpawnShowMakerTab(const FSpawnTabArgs& Args)
{
    TSharedPtr<SShowMakerWidget> ShowMakerWidget;

    TSharedRef<SDockTab> DockTab = SNew(SDockTab)
        .Label(FText::FromString("ShowMaker"))
        .TabRole(ETabRole::PanelTab)
        [
            SAssignNew(ShowMakerWidget, SShowMakerWidget)
                .EditorHelper(EditorHelper) 
                .EditShowSequencer(EditorHelper->EditShowSequencer)  // ShowSequencer 전달 (필요 시)
                .OnAddKey_Lambda([this](UShowBase* ShowBase) 
                    {
                        DetailsView->ForceRefresh();
				    })
                .OnRemoveKey_Lambda([this]()
                    {
                        DetailsView->ForceRefresh();
                    })
        ];

    EditorHelper->SetShowMakerWidget(ShowMakerWidget);

    return DockTab;
}

void FShowSequencerEditorToolkit::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UShowSequenceAsset* InShowSequenceAsset)
{
    EditorHelper = MakeShared<FShowSequencerEditorHelper>();
    EditorHelper->NewShowSequencer(InShowSequenceAsset);

    // 기본 에디터 레이아웃 설정
    const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_ShowSequencerEditor_Layout")
        ->AddArea(
            FTabManager::NewPrimaryArea()
            ->SetOrientation(Orient_Horizontal)
            ->Split
			(
				FTabManager::NewStack()
                ->AddTab("DetailsTab", ETabState::OpenedTab)
                ->AddTab("ShowMakerTab", ETabState::OpenedTab)
                ->SetForegroundTab(FName("ShowMakerTab")) // 초기 포커스 설정
                ->SetHideTabWell(false)
			)
        );

    TArray<UObject*> SingleObjectArray;
    SingleObjectArray.Add(InShowSequenceAsset);

    // 에디터 초기화
    InitAssetEditor(
        Mode,
        InitToolkitHost,
        FName(TEXT("ShowSequencerEditor")),
        Layout,
        true, // 기본 메뉴 생성
        true, // 기본 툴바 생성
        SingleObjectArray
    );

    GenerateToolbarButtons();

    GenerateExtendMenuBar();  // 메뉴바 생성

    RegenerateMenusAndToolbars();
}

void FShowSequencerEditorToolkit::GenerateToolbarButtons()
{
    TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
    ToolbarExtender->AddToolBarExtension(
        "Asset",
        EExtensionHook::After,
        GetToolkitCommands(),
        FToolBarExtensionDelegate::CreateSP(this, &FShowSequencerEditorToolkit::AddToolbarButtons)
    );

    AddToolbarExtender(ToolbarExtender);
}

void FShowSequencerEditorToolkit::AddToolbarButtons(FToolBarBuilder& ToolbarBuilder)
{
    ToolbarBuilder.BeginSection("CustomButtons");
    {
        ToolbarBuilder.AddToolBarButton(
            FUIAction(FExecuteAction::CreateSP(this, &FShowSequencerEditorToolkit::OpenSkeletalMeshPicker)),
            NAME_None,
            FText::FromString("SkeletalMesh"),
            FText::FromString("Select SkeletalMesh")
        );
    }
    {
        ToolbarBuilder.AddToolBarButton(
            FUIAction(FExecuteAction::CreateSP(this, &FShowSequencerEditorToolkit::OpenAnimPicker)),
            NAME_None,
            FText::FromString("Anim"),
            FText::FromString("Select Anim")
        );
    }
    {
        ToolbarBuilder.AddToolBarButton(
            FUIAction(FExecuteAction::CreateSP(this, &FShowSequencerEditorToolkit::OpenActorPicker)),
            NAME_None,
            FText::FromString("Actor"),
            FText::FromString("Select Actor")
        );
    }    
    ToolbarBuilder.EndSection();
}

void FShowSequencerEditorToolkit::OpenActorPicker()
{
    // 새로운 윈도우 생성
    TSharedRef<SWindow> ActorPickerWindow = SNew(SWindow)
        .Title(FText::FromString("Select Actor"))
        .ClientSize(FVector2D(600, 400));

    // Content Browser 모듈 가져오기
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    // 애셋 선택기 생성 설정
    FAssetPickerConfig AssetPickerConfig;
    // 스켈레탈 메쉬만 표시되도록 필터링 설정
    AssetPickerConfig.Filter.ClassPaths.Add(AActor::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.bRecursiveClasses = true;
    AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda([](const FAssetData& AssetData) -> bool
        {
            if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
            {
                if (Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
                {
                    UClass* ActorClass = Blueprint->GeneratedClass;
                    if (ActorClass)
                    {
                        return false;
                    }
                }
            }
            return true;
        });

    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &FShowSequencerEditorToolkit::OnSelectedActor);

    // 애셋 선택기를 새로운 윈도우에 설정
    ActorPickerWindow->SetContent(
        ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
    );

    // 윈도우를 띄움
    FSlateApplication::Get().AddWindow(ActorPickerWindow);
}

void FShowSequencerEditorToolkit::OnSelectedActor(const FAssetData& SelectedAsset)
{
    if (UBlueprint* Blueprint = Cast<UBlueprint>(SelectedAsset.GetAsset()))
    {
        if (Blueprint->GeneratedClass->IsChildOf(AActor::StaticClass()))
        {
            // 블루프린트가 AActor의 자식 클래스인지 확인
            UClass* ActorClass = Blueprint->GeneratedClass;
            if (ActorClass)
            {
                EditorHelper->ReplaceActorPreviewWorld(ActorClass);
            }
        }
    }
}

void FShowSequencerEditorToolkit::OpenSkeletalMeshPicker()
{
    // 새로운 윈도우 생성
    TSharedRef<SWindow> ActorPickerWindow = SNew(SWindow)
        .Title(FText::FromString("Select SkeletalMesh"))
        .ClientSize(FVector2D(600, 400));

    // Content Browser 모듈 가져오기
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    // 애셋 선택기 생성 설정
    FAssetPickerConfig AssetPickerConfig;
    // 스켈레탈 메쉬만 표시되도록 필터링 설정
    AssetPickerConfig.Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.bRecursiveClasses = true;
    AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda([](const FAssetData& AssetData) -> bool
        {
            return !AssetData.GetClass()->IsChildOf(USkeletalMesh::StaticClass());
        });

    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &FShowSequencerEditorToolkit::OnSelectedSkeletalMesh);

    // 애셋 선택기를 새로운 윈도우에 설정
    ActorPickerWindow->SetContent(
        ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
    );

    // 윈도우를 띄움
    FSlateApplication::Get().AddWindow(ActorPickerWindow);
}

void FShowSequencerEditorToolkit::OnSelectedSkeletalMesh(const FAssetData& SelectedAsset)
{
    USkeletalMesh* SelectedSkeletalMesh = Cast<USkeletalMesh>(SelectedAsset.GetAsset());
    if (SelectedSkeletalMesh)
	{
		EditorHelper->ReplaceSkeletalMeshPreviewWorld(SelectedSkeletalMesh);
	}
}

void FShowSequencerEditorToolkit::OpenAnimPicker()
{
    // 새로운 윈도우 생성
    TSharedRef<SWindow> ActorPickerWindow = SNew(SWindow)
        .Title(FText::FromString("Select Actor"))
        .ClientSize(FVector2D(600, 400));

    // Content Browser 모듈 가져오기
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    // 애셋 선택기 생성 설정
    FAssetPickerConfig AssetPickerConfig;
    AssetPickerConfig.Filter.ClassPaths.Add(UAnimInstance::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    AssetPickerConfig.Filter.bRecursiveClasses = true;
    AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda([](const FAssetData& AssetData) -> bool
        {
            if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
            {
                if (Blueprint->GeneratedClass->IsChildOf(UAnimInstance::StaticClass()))
                {
                    UClass* ActorClass = Blueprint->GeneratedClass;
                    if (ActorClass)
                    {
                        return false;
                    }
                }
            }
            return true;
        });

    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &FShowSequencerEditorToolkit::OnSelectedAnim);

    // 애셋 선택기를 새로운 윈도우에 설정
    ActorPickerWindow->SetContent(
        ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
    );

    // 윈도우를 띄움
    FSlateApplication::Get().AddWindow(ActorPickerWindow);
}

void FShowSequencerEditorToolkit::OnSelectedAnim(const FAssetData& SelectedAsset)
{
    if (UBlueprint* Blueprint = Cast<UBlueprint>(SelectedAsset.GetAsset()))
    {
        if (Blueprint->GeneratedClass->IsChildOf(UAnimInstance::StaticClass()))
        {
            // 블루프린트가 UAnimInstance의 자식 클래스인지 확인
            UClass* AnimInstanceClass = Blueprint->GeneratedClass;
            if (AnimInstanceClass)
            {
                EditorHelper->ReplaceAnimInstancePreviewWorld(AnimInstanceClass);
            }
        }
    }
}

void FShowSequencerEditorToolkit::GenerateExtendMenuBar()
{
    // FExtender 생성
    TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender);

    // 지역 함수 정의
    struct Local
    {
        static void ExtendMenu(FMenuBuilder& MenuBuilder, TSharedPtr<FTabManager> InTabManager)
        {
            // ShowSequencer 섹션 추가
            MenuBuilder.BeginSection("ShowSequencer", FText::FromString("ShowSequencer"));
            {
                // ShowMaker 메뉴 항목 추가
                MenuBuilder.AddMenuEntry(
                    FText::FromString("ShowMaker"),
                    FText::FromString("Open ShowMaker Window"),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([InTabManager]() {
                        // ShowMaker 창을 여는 로직
                        // 탭을 다시 열기 위한 로직
                        if (!InTabManager->FindExistingLiveTab(FName("ShowMakerTab")).IsValid())
                        {
                            InTabManager->TryInvokeTab(FName("ShowMakerTab"));
                        }
                        }))
                );

                MenuBuilder.AddMenuEntry(
                    FText::FromString("Open Details Tab"),
                    FText::FromString("Reopen the Details tab for editing."),
                    FSlateIcon(),
                    FUIAction(FExecuteAction::CreateLambda([InTabManager]() {
                        // ShowMaker 창을 여는 로직
                        // 탭을 다시 열기 위한 로직
                        if (!InTabManager->FindExistingLiveTab(FName("DetailsTab")).IsValid())
                        {
                            InTabManager->TryInvokeTab(FName("DetailsTab"));
                        }
                        }))
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
