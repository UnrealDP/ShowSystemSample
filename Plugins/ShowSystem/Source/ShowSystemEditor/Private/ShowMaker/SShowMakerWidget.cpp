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
        PreviewViewport->SpawnActorInPreviewWorld(LastSelectedActorClass);
    }
    else
    {
        PreviewViewport->SpawnActorInPreviewWorld(AActor::StaticClass());

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
                            .MinWidth(50.0f)
                            .SecondToWidthRatio(10.0f)
                            .OnAddKey_Lambda([this](FShowKey* Key)
                                {
                                    if (NotifyHookInstance)
                                    {
                                        NotifyHookInstance->MarkPackageDirty();
                                        EditorHelper->ShowSequencerDetailsViewForceRefresh();
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
    NotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(EditorHelper->EditShowSequencer));
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

void SShowMakerWidget::SetShowKey(FShowKey* NewShowKey)
{
    if (SelectedShowKey == NewShowKey)
    {
        return;
    }

    SelectedShowKey = NewShowKey;
    if (SelectedShowKey)
    {
        UScriptStruct* ScriptStruct = ShowSystem::GetShowKeyStaticStruct(SelectedShowKey->KeyType);
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(ScriptStruct, (uint8*)SelectedShowKey));
        StructureDetailsView->SetStructureData(StructData);
    }
    else
    {
        StructureDetailsView = nullptr;
    }
}