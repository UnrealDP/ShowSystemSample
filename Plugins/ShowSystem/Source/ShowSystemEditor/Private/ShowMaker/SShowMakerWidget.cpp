// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowMakerWidget.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "ActorPreviewViewport.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PathsUtil.h"
#include "RunTime/ShowPlayer.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "SVerticalResizableSplitter.h"
#include "SHorizontalResizableSplitter.h"
#include "IStructureDetailsView.h"
#include "SlateEditorUtils.h"
#include "ShowMaker/ShowSequencerNotifyHook.h"
#include "ShowSequencerEditorToolkit.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowMakerWidget::Construct(const FArguments& InArgs)
{
    ToolkitInstance = InArgs._ToolkitInstance;
    EditShowSequencer = InArgs._EditShowSequencer;

    EditorHelper = MakeShared<FShowSequencerEditorHelper>();
    EditorHelper->SetShowSequencerEditor(EditShowSequencer);

    FMenuBarBuilder MenuBarBuilder(nullptr);

    // 기본 UI 레이아웃 구성
    ChildSlot
        [
            SNew(SHorizontalResizableSplitter)
                .Widgets(
                    {
                        ConstructMainBody(),
                        ConstructShowKeyDetails()
                    }
                )
                .InitialRatios(
					{ 0.83f, 0.17f }
                )
        ];

    LoadedSkeletalMesh = CheckLoadSkeletalMesh();
    if (LoadedSkeletalMesh)
    {
        PreviewViewport->SetPreviewAsset(LoadedSkeletalMesh);
        PreviewViewport->RefreshViewport();
    }

    UWorld* PreviewWorld = PreviewViewport->GetPreviewWorld();
    if (PreviewWorld)
    {
        ShowPlayer = PreviewWorld->GetSubsystem<UShowPlayer>();
    }

    if (Actor && EditShowSequencer)
    {
        ShowPlayer->PlaySoloShow(Actor, EditShowSequencer);
    }
}

USkeletalMesh* SShowMakerWidget::CheckLoadSkeletalMesh()
{
    FString SelectedMeshPath;
    // 이전에 로드했던 스켈레털 메쉬 있는지 확인
    // 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
    // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
    if (GConfig)
    {
        GConfig->GetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), SelectedMeshPath, *ConfigFilePath);
    }

    USkeletalMesh* SkeletalMesh = nullptr;
    if (SelectedMeshPath.IsEmpty())
    {
        // 디폴트 어셋 로드
        SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/EngineMeshes/SkeletalCube.SkeletalCube"));

        SelectedMeshPath = SkeletalMesh->GetPathName();
        // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
        if (GConfig)
        {
            GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedMeshPath, *ConfigFilePath);
            GConfig->Flush(false, *ConfigFilePath);
        }
    }
	else
	{
        SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *SelectedMeshPath);
	}

    return SkeletalMesh;
}

TSharedRef<SWidget> SShowMakerWidget::ConstructMainBody()
{
    return SNew(SVerticalBox)
        // 메뉴 버튼 추가
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SMenuAnchor)
                .OnGetMenuContent(this, &SShowMakerWidget::GenerateMenuContent)  // 메뉴 생성 함수 연결
                [
                    SNew(SButton)
                        .Text(FText::FromString("Select Skeletal Mesh"))
                        .OnClicked(this, &SShowMakerWidget::OnMenuButtonClicked)  // 메뉴 버튼 클릭 시 메뉴를 보여줌
                ]
        ]

        // 메뉴바 추가
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            CreateMenuBar()
        ]

        // 여기에 다른 위젯들을 추가해 편집 창을 완성
        + SVerticalBox::Slot()
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            SNew(SVerticalResizableSplitter)
                .Widgets
                (
                    {
                        ConstructPreviewScenePanel(),
                        SNew(SShowKeyBoxHandler)
                            .ShowSequencerEditorHelper(EditorHelper)
                            .Height(20.0f)
                            .MinWidth(50.0f)
                            .SecondToWidthRatio(10.0f)
                            .OnAddKey_Lambda([this](FShowKey* Key)
                                {
                                    if (NotifyHookInstance)
                                    {
                                        NotifyHookInstance->MarkPackageDirty();
                                        if (ToolkitInstance && ToolkitInstance->DetailsView && EditorHelper->EditShowSequencer)
                                        {
                                            ToolkitInstance->DetailsView->ForceRefresh();
                                        }
                                    }
                                })
                            .OnRemoveKey_Lambda([](FShowKey* Key) {})
                            .OnClickedKey(this, &SShowMakerWidget::SetShowKey)
                            .OnChangedKey_Lambda([this](FShowKey* Key) 
                                {
                                    if (NotifyHookInstance)
                                    {
                                        NotifyHookInstance->MarkPackageDirty();
                                    }
                                })
                    }
                )
                .InitialRatios
                (
                    {
                        0.7f,
                        0.3f
                    }
                )
        ]

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .AutoHeight()
        [
            SNew(SShowSequencerScrubPanel)
                .ShowSequencerEditorHelper(EditorHelper)
                .bDisplayAnimScrubBarEditing(true)
                .bAllowZoom(true)
        ];
}

TSharedRef<SWidget> SShowMakerWidget::GenerateMenuContent()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    // 첫 번째 메쉬 선택 항목
    MenuBuilder.AddMenuEntry(
        FText::FromString("Default Skeletal Mesh"),
        FText::FromString("Select the Default Skeletal Mesh"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &SShowMakerWidget::OpenSkeletalMeshPicker))
    );

    return MenuBuilder.MakeWidget();
}

FReply SShowMakerWidget::OnMenuButtonClicked()
{
    // 메뉴를 열 때 호출되며, 버튼 클릭 시 메뉴를 보여줌
    return FReply::Handled();
}

TSharedRef<SWidget> SShowMakerWidget::CreateMenuBar()
{
    FMenuBarBuilder MenuBarBuilder(nullptr);

    // "Select Skeletal Mesh" 풀다운 메뉴 추가
    MenuBarBuilder.AddPullDownMenu(
        FText::FromString("Select Skeletal Mesh"),
        FText::FromString("Select a Skeletal Mesh from the list"),
        FNewMenuDelegate::CreateRaw(this, &SShowMakerWidget::GenerateMenu)
    );

    return MenuBarBuilder.MakeWidget();
}

void SShowMakerWidget::GenerateMenu(FMenuBuilder& MenuBuilder)
{
    // 첫 번째 메쉬 선택 항목
    MenuBuilder.AddMenuEntry(
        FText::FromString("Select Skeletal Mesh"),
        FText::FromString("Select the Default Skeletal Mesh"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &SShowMakerWidget::OpenSkeletalMeshPicker))
    );
}

void SShowMakerWidget::OpenSkeletalMeshPicker()
{
    // 새로운 윈도우 생성
    SkeletalMeshPickerWindow = SNew(SWindow)
        .Title(FText::FromString("Select Skeletal Mesh"))
        .ClientSize(FVector2D(600, 400));

    // Content Browser 모듈 가져오기
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    // 애셋 선택기 생성 설정
    FAssetPickerConfig AssetPickerConfig;
    // 스켈레탈 메쉬만 표시되도록 필터링 설정
    AssetPickerConfig.Filter.ClassPaths.Add(USkeletalMesh::StaticClass()->GetClassPathName());
    /*AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateLambda([](const FAssetData& AssetData) -> bool
        {
            return AssetData.GetClass()->IsChildOf(USkeletalMesh::StaticClass());
        });*/
    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SShowMakerWidget::OnSkeletalMeshSelected);

    // 애셋 선택기를 새로운 윈도우에 설정
    SkeletalMeshPickerWindow->SetContent(
        ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
    );

    // 윈도우를 띄움
    FSlateApplication::Get().AddWindow(SkeletalMeshPickerWindow.ToSharedRef());
}

void SShowMakerWidget::OnSkeletalMeshSelected(const FAssetData& SelectedAsset)
{
    // 선택된 스켈레탈 메쉬 로드
    USkeletalMesh* SelectedSkeletalMesh = Cast<USkeletalMesh>(SelectedAsset.GetAsset());

    if (SelectedSkeletalMesh)
    {
        UE_LOG(LogTemp, Log, TEXT("Selected Skeletal Mesh: %s"), *SelectedSkeletalMesh->GetName());

        LoadedSkeletalMesh = SelectedSkeletalMesh;
        PreviewViewport->SetPreviewAsset(LoadedSkeletalMesh);

        FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowSystem"), TEXT("Config/ShowSystemEditor.ini"));
        // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
        if (GConfig)
        {
            FString SelectedSkeletalMeshPath = SelectedSkeletalMesh->GetPathName();
            GConfig->SetString(TEXT("SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedSkeletalMeshPath, *ConfigFilePath);
            GConfig->Flush(false, *ConfigFilePath);
        }
    }

    if (SkeletalMeshPickerWindow.IsValid())
    {
        // 윈도우를 닫음
        FSlateApplication::Get().RequestDestroyWindow(SkeletalMeshPickerWindow.ToSharedRef());
        SkeletalMeshPickerWindow.Reset();
    }
}

TSharedRef<SWidget> SShowMakerWidget::ConstructPreviewScenePanel()
{
    PreviewViewport = SNew(SActorPreviewViewport);
    PreviewViewport->SpawnActorInPreviewWorld(AActor::StaticClass());
    Actor = PreviewViewport->GetActor();

    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            PreviewViewport.ToSharedRef()
        ];
}

TSharedRef<SWidget> SShowMakerWidget::ConstructShowKeyDetails()
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs SkillShowDetailsViewArgs;
    SkillShowDetailsViewArgs.bAllowSearch = true;
    SkillShowDetailsViewArgs.bShowOptions = true;
    NotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(EditShowSequencer));
    SkillShowDetailsViewArgs.NotifyHook = NotifyHookInstance.Get();

    FStructureDetailsViewArgs SkillShowDetailsArgs;
    SkillShowDetailsArgs.bShowObjects = true;
    
    StructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
        SkillShowDetailsViewArgs,
        SkillShowDetailsArgs,
        nullptr,
        FText::FromString("Skill Key Details")
    );

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SMenuAnchor)
                .OnGetMenuContent(this, &SShowMakerWidget::GenerateMenuContent)
                [
                    SNew(SButton)
                        .Text(FText::FromString("Show Key"))
                ]
        ]

        SLATE_VERTICAL_SLOT(0, 5)

    + SVerticalBox::Slot()
        .AutoHeight()
        [
            StructureDetailsView->GetWidget().ToSharedRef()
        ];
}

void SShowMakerWidget::SetShowKey(FShowKey* NewShowKey)
{
    if (SelectedShowKey == NewShowKey)
    {
        return;
    }

    SelectedShowKey = NewShowKey;
    if (SelectedShowKey)
    {
        // 구조체 데이터
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(FShowKey::StaticStruct(), (uint8*)SelectedShowKey));
        StructureDetailsView->SetStructureData(StructData);
    }
    else
    {
        StructureDetailsView = nullptr;
    }
}

void SShowMakerWidget::RefreshPreviewViewport()
{
    if (PreviewViewport.IsValid())
    {
        PreviewViewport->RefreshViewport();
    }
}

bool SShowMakerWidget::SetPreviewAsset(UObject* InAsset)
{
    if (PreviewViewport.IsValid())
    {
        return PreviewViewport->SetPreviewAsset(InAsset);
    }
    return false;
}

void SShowMakerWidget::UpdatePreviewViewportsVisibility()
{
    /*PreviewViewport->SetVisibility(EVisibility::Collapsed);
    PreviewViewport->SetVisibility(EVisibility::Visible);*/
}

void SShowMakerWidget::MakeFileMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString("Open"),
        FText::FromString("Open a file"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &SShowMakerWidget::OnOpenFile))
    );
}

void SShowMakerWidget::OnOpenFile()
{
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
