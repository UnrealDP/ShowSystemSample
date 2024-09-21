// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowMakerWidget.h"
#include "SlateOptMacros.h"
#include "PersonaModule.h"
#include "IPersonaToolkit.h"
#include "RunTime/ShowSequencer.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "Factories/SkeletonFactory.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AdvancedPreviewScene.h"
#include "Widgets/SViewport.h"
#include "ActorPreviewViewportClient.h"
#include "Slate/SceneViewport.h"
#include "SEditorViewport.h"
#include "ActorPreviewViewport.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowMakerWidget::Construct(const FArguments& InArgs)
{
    EditShowSequencer = InArgs._EditShowSequencer;

    /*FPersonaToolkitArgs PersonaToolkitArgs;
    PersonaToolkitArgs.OnPreviewSceneSettingsCustomized = FOnPreviewSceneSettingsCustomized::FDelegate::CreateSP(this, &SShowMakerWidget::HandleOnPreviewSceneSettingsCustomized);
    PersonaToolkitArgs.OnPreviewSceneCreated = FOnPreviewSceneCreated::FDelegate::CreateSP(this, &SShowMakerWidget::HandlePreviewSceneCreated);
    PersonaToolkitArgs.bPreviewMeshCanUseDifferentSkeleton = true;

    FPersonaModule& PersonaModule = FModuleManager::GetModuleChecked<FPersonaModule>("Persona");
    PersonaToolkit = PersonaModule.CreatePersonaToolkit(LoadedSkeletalMesh, PersonaToolkitArgs);*/
    //PersonaToolkit = PersonaModule.CreatePersonaToolkit(EditShowSequencer, PersonaToolkitArgs);

    //PreviewScenePtr = GetPersonaToolkit()->GetPreviewScene();
    //if (TSharedPtr<IPersonaPreviewScene> PreviewScene = PreviewScenePtr.Pin())
    //{
    //    PreviewScene->SetDefaultAnimationMode(EPreviewSceneDefaultAnimationMode::Animation);
    //}
    //GetPersonaToolkit()->GetPreviewScene()->SetActor

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
                    SpawnWidget_Preview()
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

    //ChildSlot
    //    [
    //        SNew(SVerticalBox)
    //            + SVerticalBox::Slot()
    //            .AutoHeight()
    //            [
    //                // 메뉴 바 추가
    //                FMenuBarBuilder::AddPullDownMenu(
    //                        FText::FromString("File"),
    //                        FText::FromString("Open file menu"),
    //                        FNewMenuDelegate::CreateSP(this, &SShowMakerWidget::MakeFileMenu)
    //                    )
    //            ]

    //            + SVerticalBox::Slot()
    //            .AutoHeight()
    //            [
    //                // 툴바 추가
    //                FToolBarBuilder::MakeToolBar()
    //                    .AddToolBarButton(
    //                        FUIAction(FExecuteAction::CreateSP(this, &SShowMakerWidget::OnButtonClick)),
    //                        NAME_None,
    //                        FText::FromString("Save"),
    //                        FText::FromString("Save the current show"),
    //                        FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "Icons.Save")
    //                    )
    //            ]

    //            + SVerticalBox::Slot()
    //            [
    //                // 실제 위젯 콘텐츠
    //                SNew(STextBlock)
    //                    .Text(FText::FromString("Show Maker Content Goes Here"))
    //            ]
    //    ];


    /*LoadedWorld = CheckLoadWorld();
    if (LoadedWorld)
    {
        PreviewScene->SetPreviewWorld(LoadedWorld);
    }

    LoadedSkeletalMesh = CheckLoadSkeletalMesh();
    if (LoadedSkeletalMesh)
    {
        PreviewScene->SetPreviewSkeletalMesh(LoadedSkeletalMesh);
    }*/

    LoadedSkeletalMesh = CheckLoadSkeletalMesh();
    if (LoadedSkeletalMesh)
    {
        PreviewViewport->SetPreviewAsset(LoadedSkeletalMesh);
        PreviewViewport->RefreshViewport();
    }

    // 초기 액터 또는 메쉬를 프리뷰 씬에 추가
    //PreviewScene->SetPreviewActor(Actor);
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

void SShowMakerWidget::HandleOnPreviewSceneSettingsCustomized(IDetailLayoutBuilder& DetailBuilder)
{
    //DetailBuilder.HideCategory("Mesh");
    //DetailBuilder.HideCategory("Physics");
    // in mesh editor, we hide preview mesh section and additional mesh section
    // sometimes additional meshes are interfering with preview mesh, it is not a great experience
    //DetailBuilder.HideCategory("Additional Meshes");
}


void SShowMakerWidget::HandlePreviewSceneCreated(const TSharedRef<IPersonaPreviewScene>& InPreviewScene)
{
    /*AAnimationEditorPreviewActor* Actor = InPreviewScene->GetWorld()->SpawnActor<AAnimationEditorPreviewActor>(AAnimationEditorPreviewActor::StaticClass(), FTransform::Identity);
    Actor->SetFlags(RF_Transient);
    InPreviewScene->SetActor(Actor);

    UDebugSkelMeshComponent SkeletalMeshComponent = NewObject<UDebugSkelMeshComponent>(Actor);
    if (GEditor->PreviewPlatform.GetEffectivePreviewFeatureLevel() <= ERHIFeatureLevel::ES3_1)
    {
        SkeletalMeshComponent->SetMobility(EComponentMobility::Static);
    }
    SkeletalMeshComponent->bSelectable = false;
    SkeletalMeshComponent->MarkRenderStateDirty();

    SkeletalMeshComponent->SetMeshDeformer(DeformerObject);

    InPreviewScene->AddComponent(SkeletalMeshComponent, FTransform::Identity);
    InPreviewScene->SetPreviewMeshComponent(SkeletalMeshComponent);

    InPreviewScene->SetAllowMeshHitProxies(false);
    InPreviewScene->SetAdditionalMeshesSelectable(false);*/
}


TSharedRef<SDockTab> SShowMakerWidget::SpawnTab_Preview()
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


TSharedRef<SWidget> SShowMakerWidget::SpawnWidget_Preview()
{
    PreviewViewport = SNew(SActorPreviewViewport);

    // 일반 위젯을 리턴하여 다른 Slate 구조 내에서 사용 가능하게 설정
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            PreviewViewport.ToSharedRef()
        ];
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

TSharedRef<SWidget> SShowMakerWidget::ConstructPreviewScenePanel(bool bDisplayAnimScrubBarEditing)
{
    return SNew(STextBlock)
        .Text(FText::FromString("ShowMaker Editor"));

    if (PreviewViewport.IsValid())
    {
        /*return SAssignNew(ShowSequencerScrubPanel, SShowSequencerScrubPanel, PreviewScenePtr.Pin().ToSharedRef())
            .EditShowSequencer(EditShowSequencer)
            .ViewInputMin(this, &SShowMakerWidget::GetViewMinInput)
            .ViewInputMax(this, &SShowMakerWidget::GetViewMaxInput)
            .bDisplayAnimScrubBarEditing(bDisplayAnimScrubBarEditing)
            .OnSetInputViewRange(this, &SShowMakerWidget::SetInputViewRange)
            .bAllowZoom(true);*/
    }

    //// FActorPreviewScene 생성
    //FAdvancedPreviewScene::ConstructionValues CVS = FAdvancedPreviewScene::ConstructionValues();
    //PreviewScene = MakeShareable(new FActorPreviewScene(CVS));

    //// Viewport Client 생성 (카메라 및 입력 처리 담당)
    //TSharedPtr<class FActorPreviewViewportClient> ViewportClient = MakeShareable(new FActorPreviewViewportClient(PreviewScene.Get(), nullptr));

    //// SceneViewport 생성
    //TSharedPtr<FSceneViewport> SceneViewport = MakeShareable(new FSceneViewport(ViewportClient.Get(), nullptr));

    //// SViewport 위젯 생성 (뷰포트와 Slate UI 연결)
    //ViewportWidget = SNew(SViewport)
    //    .EnableGammaCorrection(false)
    //    .ShowEffectWhenDisabled(false);

    //// Viewport와 SceneViewport를 연결
    //ViewportWidget->SetViewportInterface(SceneViewport.ToSharedRef());

    //// 뷰포트 위젯을 반환
    //return SNew(SBox)
    //    .WidthOverride(800)  // 뷰포트 크기 설정
    //    .HeightOverride(600)
    //    [
    //        ViewportWidget.ToSharedRef()
    //    ];

    

    //return SNullWidget::NullWidget;
}


void SShowMakerWidget::SetInputViewRange(float InViewMinInput, float InViewMaxInput)
{
    ViewMaxInput = FMath::Min<float>(InViewMaxInput, 0.f);
    ViewMinInput = FMath::Max<float>(InViewMinInput, 0.f);
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
