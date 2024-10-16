// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowMakerWidget.h"
#include "ActorPreviewViewport.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Misc/PathsUtil.h"
#include "RunTime/ShowPlayer.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "SVerticalResizableSplitter.h"
#include "SHorizontalResizableSplitter.h"
#include "IStructureDetailsView.h"
#include "SlateEditorUtils.h"
#include "ShowMaker/ShowSequencerNotifyHook.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowMaker/SShowSequencerEditor.h"
#include "Runtime/ShowSystem.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowMakerWidget::Construct(const FArguments& InArgs)
{
    EditorHelper = InArgs._EditorHelper;
    OnAddKey = InArgs._OnAddKey;
    OnRemoveKey = InArgs._OnRemoveKey;

    ShowSequencerState = TAttribute<EShowSequencerState>::Create(TAttribute<EShowSequencerState>::FGetter::CreateLambda([this]()
        {
            return EditorHelper->EditShowSequencer->GetShowSequencerState();
        }));

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


    if (UClass* LastSelectedActorClass = EditorHelper->GetLastSelectedActorClass())
    {
        AActor* Owner = PreviewViewport->SpawnActorInPreviewWorld(LastSelectedActorClass);
        EditorHelper->EditShowSequencer->EditorSetOwner(Owner);
    }
    else
    {
        AActor* Owner = PreviewViewport->SpawnActorInPreviewWorld(AActor::StaticClass());
        EditorHelper->EditShowSequencer->EditorSetOwner(Owner);

        USkeletalMesh* LoadedSkeletalMesh = EditorHelper->LoadLastSelectedOrDefaultSkeletalMesh();
        if (LoadedSkeletalMesh)
        {
            PreviewViewport->SetSkeletalMesh(LoadedSkeletalMesh);
            PreviewViewport->RefreshViewport();
        }
    }

    if (UClass* LastSelectedAnimInstanceClass = EditorHelper->GetLastSelectedAnimInstanceClass())
    {
        PreviewViewport->SetAnimInstance(LastSelectedAnimInstanceClass);
    }
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

UWorld* SShowMakerWidget::GetPreviewWorld() const
{
    if (!PreviewViewport.IsValid())
    {
        return nullptr;
    }
    return PreviewViewport->GetPreviewWorld();
}

TSharedRef<SWidget> SShowMakerWidget::ConstructMainBody()
{
    return SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            SNew(SVerticalResizableSplitter)
                .Widgets
                (
                    {
                        ConstructPreviewScenePanel(),

                        SNew(SShowSequencerEditor)
                            .EditorHelper(EditorHelper)
                            .Height(20.0f)
                            .MinWidth(20.0f)
                            .ShowSequencerState(ShowSequencerState)
                            .OnAddKey_Lambda([this](UShowBase* ShowBase)
                                {
                                    EditorHelper->EditShowSequencer->MarkPackageDirty();

                                    if (OnAddKey.IsBound())
									{
										OnAddKey.Execute(ShowBase);
									}
                                })
                            .OnRemoveKey_Lambda([this]()
                                {
                                    TObjectPtr<UShowBase> CheckSelectedShowBase = EditorHelper->CheckGetSelectedShowBase();
                                    if (CheckSelectedShowBase != EditorHelper->SelectedShowBase)
                                    {
                                        EditorHelper->SelectedShowBase = CheckSelectedShowBase;
                                        UpdateShowKeyDetails();
                                    }
                                    
                                    if (OnRemoveKey.IsBound())
                                    {
                                        OnRemoveKey.Execute();
                                    }
                                })
                            .OnClickedKey_Lambda([this](UShowBase* ShowBase)
								{
                                    if (EditorHelper->SelectedShowBase != ShowBase)
                                    {
                                        EditorHelper->SelectedShowBase = ShowBase;
                                        UpdateShowKeyDetails();
                                    }
								})
                            .OnChangedKey_Lambda([this](UShowBase* ShowBase)
                                {
                                    EditorHelper->EditShowSequencer->MarkPackageDirty();
                                })
                            .OnKeyDownSpace_Lambda([this]()
                                {
                                    EditorHelper->Play();
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
        ];
}

TSharedRef<SWidget> SShowMakerWidget::ConstructPreviewScenePanel()
{
    PreviewViewport = SNew(SActorPreviewViewport);
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
    NotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(EditorHelper.Get()));
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
            SNew(SBorder)
                .Padding(FMargin(10.0f, 5.0f))
                .BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder")) // 기본 경계 스타일
                .HAlign(HAlign_Left)
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString("Show Key"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
                        .Justification(ETextJustify::Left)
                        .ColorAndOpacity(FLinearColor::White)
                ]
        ]

        SLATE_VERTICAL_SLOT(0, 5)

    + SVerticalBox::Slot()
        .AutoHeight()
        [
            StructureDetailsView->GetWidget().ToSharedRef()
        ];
}

void SShowMakerWidget::UpdateShowKeyDetails()
{
    if (EditorHelper->SelectedShowBase)
    {
        UScriptStruct* ScriptStruct = EditorHelper->GetShowKeyStaticStruct(EditorHelper->SelectedShowBase);
        FShowKey* ShowKeyPtr = EditorHelper->GetMutableShowKey(EditorHelper->SelectedShowBase);
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(ScriptStruct, (uint8*)ShowKeyPtr));
        StructureDetailsView->SetStructureData(StructData);
    }
    else
    {
        StructureDetailsView->SetStructureData(nullptr);
    }
}