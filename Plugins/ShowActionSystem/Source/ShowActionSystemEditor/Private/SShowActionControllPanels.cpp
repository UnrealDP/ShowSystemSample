// Fill out your copyright notice in the Description page of Project Settings.


#include "SShowActionControllPanels.h"
#include "SlateOptMacros.h"
#include "Data/SkillData.h"
#include "SHorizontalResizableSplitter.h"
#include "ShowMaker/SShowSequencerControllPanel.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ActionBase.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "ShowMaker/SShowSequencerScrubBoard.h"
#include "ShowMaker/SShowSequencerEditHeader.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowActionControllPanels::Construct(const FArguments& InArgs)
{
    Args = InArgs;
    OnAddKey = InArgs._OnAddKey;
    OnSelectedKey = InArgs._OnSelectedKey;
    OnRemoveKey = InArgs._OnRemoveKey;

    ShowSequencerState = TAttribute<EShowSequencerState>::Create(TAttribute<EShowSequencerState>::FGetter::CreateLambda([this]()
        {
            return EShowSequencerState::ShowSequencer_Wait;
        }));

    ChildSlot
        [
            SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .Padding(2.0f)
                .FillHeight(1.0f)
                [
                    SNew(SHorizontalResizableSplitter)
                        .Widgets(
                            {
                                ConstructLeftWidget(InArgs),
                                SAssignNew(HorizontalBox, SHorizontalBox)
                            }
                        )
                        .InitialRatios(
                            { 0.1f, 0.9f }
                        )
                ]
        ];

    ConstructRightWidget(InArgs);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


TSharedRef<SWidget> SShowActionControllPanels::ConstructLeftWidget(const FArguments& InArgs)
{
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SBox)
                [
                    SAssignNew(ShowSequencerEditHeader, SShowSequencerEditHeader)
                        .TitleHeight(30)
                        .Height(20)
                        .Width(100)
                        .OnAddShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBasePtr)
                            {
                                if (OnAddKey.IsBound())
                                {
                                    OnAddKey.Execute(EditorHelper, ShowBasePtr);
                                }
                                IsUpdateKey = true;
                            })
                        .OnRemoveShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
                            {
                                if (OnRemoveKey.IsBound())
                                {
                                    OnRemoveKey.Execute(EditorHelper);
                                }
                                IsUpdateKey = true;
                            })
                ]
        ]

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .AutoHeight()
        [
            SNew(SShowSequencerControllPanel)
                .ShowSequencerState(ShowSequencerState)
                .OnPlay(InArgs._OnPlay)
        ];
}

void SShowActionControllPanels::ConstructRightWidget(const FArguments& InArgs)
{
    HorizontalBox->ClearChildren();

    int32 SlotCount = ShowSequencerEditorHelperSortMapPtr ? ShowSequencerEditorHelperSortMapPtr->Num() : 0;
    if (SlotCount == 0)
    {
		return;
	}

    const float EqualWidth = 1.0f / SlotCount;

    if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Cast"))
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Cast"])
            ];
    }

    if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Exec"))
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Exec"])
            ];
    }

    if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Finish"))
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Finish"])
            ];
    }
}

TSharedRef<SWidget> SShowActionControllPanels::ConstructShowSequencerWidget(const FArguments& InArgs, TSharedPtr<FShowSequencerEditorHelper>& EditorHelper)
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SShowSequencerScrubBoard)
                .Height(30)
                .TotalValue_Lambda([EditorHelper]() { return EditorHelper->GetWidgetLengthAlignedToInterval(2.0f); })
                .CrrValue_Lambda([EditorHelper]() { return EditorHelper->EditShowSequencerPtr->GetPassedTime(); })
                .OnValueChanged_Lambda([EditorHelper](float InValue)
					{
						//EditorHelper->EditShowSequencer->SetPassedTime(InValue);
					})
        ]
        + SOverlay::Slot()
        [
            SNew(SBox)
                .Padding(FMargin(0, 30, 0, 0))
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)
                [
                    SAssignNew(ShowKeyBoxHandler, SShowKeyBoxHandler)
                        .EditorHelper(EditorHelper)
                        .Height(20)
                        .MinWidth(20)
                        .OnClickedKey_Lambda([this, EditorHelper](UShowBase* ShowBasePtr)
                            {
                                if (OnSelectedKey.IsBound())
								{
                                    OnSelectedKey.Execute(EditorHelper, ShowBasePtr);
								}
                            })
                        .OnChangedKey_Lambda([EditorHelper](UShowBase* ShowBasePtr)
                            {
                                EditorHelper->EditShowSequencerPtr->MarkPackageDirty();
                            })
                ]
        ];
}

void SShowActionControllPanels::RefreshShowActionControllPanels(TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr)
{
    ShowSequencerEditorHelperSortMapPtr = InShowSequencerEditorHelperSortMapPtr;
    ConstructRightWidget(Args);
    ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(ShowSequencerEditorHelperSortMapPtr);
}

void SShowActionControllPanels::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    // Tick 함수 내에서 매 프레임 처리할 작업 수행
    if (IsUpdateKey)
    {
        if (ShowKeyBoxHandler)
        {
            ShowKeyBoxHandler->RefreshShowKeyWidgets();
        }
        if (ShowSequencerEditHeader)
        {
            ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(ShowSequencerEditorHelperSortMapPtr);
        }
        IsUpdateKey = false;
    }
}