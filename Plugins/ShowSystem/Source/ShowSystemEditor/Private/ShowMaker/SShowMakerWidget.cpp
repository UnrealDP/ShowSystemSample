// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowMakerWidget.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "ActorPreviewViewport.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowMakerWidget::Construct(const FArguments& InArgs)
{
    EditShowSequencer = InArgs._EditShowSequencer;

    FMenuBarBuilder MenuBarBuilder(nullptr);

    // 기본 UI 레이아웃 구성
    ChildSlot
        [
            SNew(SVerticalBox)
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
                    ConstructPreviewScenePanel()
                ]
        ];

    if (EditShowSequencer)
    {
        // ShowSequencer로부터 데이터를 받아 초기화
    }
    else
    {
        // 기본값으로 초기화
    }

    /*LoadedWorld = CheckLoadWorld();
    if (LoadedWorld)
    {
        PreviewScene->SetPreviewWorld(LoadedWorld);
    }*/

    LoadedSkeletalMesh = CheckLoadSkeletalMesh();
    if (LoadedSkeletalMesh)
    {
        PreviewViewport->SetPreviewAsset(LoadedSkeletalMesh);
        PreviewViewport->RefreshViewport();
    }
}

UWorld* SShowMakerWidget::CheckLoadWorld()
{
    FString SelectedWorldPath;
    // 이전에 로드했던 스켈레털 메쉬 있는지 확인
    GConfig->GetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedWorld"), SelectedWorldPath, GEditorPerProjectIni);

    UWorld* World = nullptr;
    if (SelectedWorldPath.IsEmpty())
    {
        // 디폴트 어셋 로드
        //World = LoadObject<UWorld>(nullptr, TEXT("/Engine/EngineMeshes/SkeletalCube.SkeletalCube"));
        //SelectedWorldPath = World->GetPathName();
        //GConfig->SetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedWorld"), *SelectedWorldPath, GEditorPerProjectIni);
    }
    else
    {
        World = LoadObject<UWorld>(nullptr, *SelectedWorldPath);
    }

    return World;
}

USkeletalMesh* SShowMakerWidget::CheckLoadSkeletalMesh()
{
    FString SelectedMeshPath;
    // 이전에 로드했던 스켈레털 메쉬 있는지 확인
    GConfig->GetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), SelectedMeshPath, GEditorPerProjectIni);

    USkeletalMesh* SkeletalMesh = nullptr;
    if (SelectedMeshPath.IsEmpty())
    {
        // 디폴트 어셋 로드
        SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/EngineMeshes/SkeletalCube.SkeletalCube"));
        SelectedMeshPath = SkeletalMesh->GetPathName();
        GConfig->SetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedMeshPath, GEditorPerProjectIni);
    }
	else
	{
        SkeletalMesh = LoadObject<USkeletalMesh>(nullptr, *SelectedMeshPath);
	}

    return SkeletalMesh;
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

    // 두 번째 메쉬 선택 항목
    MenuBuilder.AddMenuEntry(
        FText::FromString("Alternate Skeletal Mesh"),
        FText::FromString("Select an Alternate Skeletal Mesh"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &SShowMakerWidget::OpenWorldPicker))
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

    // 두 번째 메쉬 선택 항목
    MenuBuilder.AddMenuEntry(
        FText::FromString("Select World"),
        FText::FromString("Select the World"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &SShowMakerWidget::OpenWorldPicker))
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
    AssetPickerConfig.Filter.ClassNames.Add(USkeletalMesh::StaticClass()->GetFName());
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
        GConfig->SetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedSkeletalMesh"), *SelectedSkeletalMesh->GetPathName(), GEditorPerProjectIni);
    }

    if (SkeletalMeshPickerWindow.IsValid())
    {
        // 윈도우를 닫음
        FSlateApplication::Get().RequestDestroyWindow(SkeletalMeshPickerWindow.ToSharedRef());
        SkeletalMeshPickerWindow.Reset();
    }
}

void SShowMakerWidget::OpenWorldPicker()
{
    // 새로운 윈도우 생성
    WorldPickerWindow = SNew(SWindow)
        .Title(FText::FromString("Select World"))
        .ClientSize(FVector2D(600, 400));

    // Content Browser 모듈 가져오기
    FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

    // 애셋 선택기 생성 설정
    FAssetPickerConfig AssetPickerConfig;
    AssetPickerConfig.Filter.ClassNames.Add(UWorld::StaticClass()->GetFName());
    AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SShowMakerWidget::OnWorldSelected);

    // 애셋 선택기를 새로운 윈도우에 설정
    WorldPickerWindow->SetContent(
        ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
    );

    // 윈도우를 띄움
    FSlateApplication::Get().AddWindow(WorldPickerWindow.ToSharedRef());
}

void SShowMakerWidget::OnWorldSelected(const FAssetData& SelectedAsset)
{
    if (UWorld* SelectedWorld = Cast<UWorld>(SelectedAsset.GetAsset()))
    {
        //PreviewViewport->SetPreviewWorld(SelectedWorld);
        GConfig->SetString(TEXT("/Script/ShowSystemEditor.SShowMakerWidget"), TEXT("LastSelectedWorld"), *SelectedWorld->GetPathName(), GEditorPerProjectIni);
    }

    if (WorldPickerWindow.IsValid())
    {
        // 윈도우를 닫음
        FSlateApplication::Get().RequestDestroyWindow(WorldPickerWindow.ToSharedRef());
        WorldPickerWindow.Reset();
    }
}

TSharedRef<SDockTab> SShowMakerWidget::ConstructPreviewScenePanel()
{
    PreviewViewport = SNew(SActorPreviewViewport);

    TSharedRef<SDockTab> SpawnedTab =
        SNew(SDockTab)
        .Label(FText::FromString("Viewport"))
        [
            SNew(SOverlay)
                + SOverlay::Slot()
                [
                    PreviewViewport.ToSharedRef()
                ]
        ];

    PreviewViewport->OnAddedToTab(SpawnedTab);

    return SpawnedTab;
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
