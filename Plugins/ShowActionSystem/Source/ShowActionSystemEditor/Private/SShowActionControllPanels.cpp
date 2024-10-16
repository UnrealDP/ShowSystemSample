// Fill out your copyright notice in the Description page of Project Settings.


#include "SShowActionControllPanels.h"
#include "SlateOptMacros.h"
#include "Data/SkillData.h"
#include "SHorizontalResizableSplitter.h"
#include "ShowMaker/SShowSequencerControllPanel.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ActionBase.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "ShowMaker/SShowSequencerScrubBoard.h"
#include "ShowMaker/SShowSequencerEditHeader.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowActionControllPanels::Construct(const FArguments& InArgs)
{
    Args = InArgs;

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
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            SNew(SBox)
                [
                    SAssignNew(ShowSequencerEditHeader, SShowSequencerEditHeader)
                        .Height(30)
                        .Width(100)
                        .OnAddShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBase) 
                            {
                            })
                        .OnRemoveShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
                            {
                            })
                ]
        ]

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .AutoHeight()
        [
            SNew(SShowSequencerControllPanel)
                .ShowSequencerState(ShowSequencerState)
                .OnPlay_Lambda([this]()
					{
						//EditorHelper->Play();
					})
        ];
}

void SShowActionControllPanels::ConstructRightWidget(const FArguments& InArgs)
{
    HorizontalBox->ClearChildren();

    int32 SlotCount = 0;
    if (CastEditorHelper)
    {
		SlotCount++;
	}
    if (ExecEditorHelper)
    {
        SlotCount++;
    }
    if (FinishEditorHelper)
    {
		SlotCount++;
	}

    if (SlotCount == 0)
    {
		return;
	}

    const float EqualWidth = 1.0f / SlotCount;

    if (CastEditorHelper)
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, CastEditorHelper)
            ];
    }

    if (ExecEditorHelper)
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, ExecEditorHelper)
            ];
    }

    if (FinishEditorHelper)
    {
        HorizontalBox->AddSlot()
            .FillWidth(EqualWidth)
            .HAlign(HAlign_Fill)
            [
                ConstructShowSequencerWidget(InArgs, FinishEditorHelper)
            ];
    }
}

TSharedRef<SWidget> SShowActionControllPanels::ConstructShowSequencerWidget(const FArguments& InArgs, TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SShowSequencerScrubBoard)
                .Height(30)
                .TotalValue_Lambda([EditorHelper]() { return EditorHelper->EditShowSequencer->GetWidgetLengthAlignedToInterval(2.0f); })
                .CrrValue_Lambda([EditorHelper]() { return EditorHelper->EditShowSequencer->GetPassedTime(); })
        ]
        + SOverlay::Slot()
        [
            SNew(SBox)
                .Padding(FMargin(0, 30, 0, 0))
                .HAlign(HAlign_Left)
                .VAlign(VAlign_Top)
                [
                    SNew(SShowKeyBoxHandler)
                        .EditorHelper(EditorHelper)
                        .Height(30)
                        .MinWidth(100)
                        .OnClickedKey_Lambda([this](UShowBase* ShowBase) {})
                        .OnChangedKey_Lambda([this](UShowBase* ShowBase) {})
                ]
        ];

    //return SNew(SVerticalBox)
    //    + SVerticalBox::Slot()
    //    .Padding(2.0f)
    //    .FillHeight(1.0f)
    //    [
    //        SNew(SShowKeyBoxHandler)
    //            .ShowSequencerEditorHelper(EditorHelper)
    //            .Height(30)
    //            .MinWidth(100)
    //            .InWidthRate_Lambda([this]() { return 1.0f; })
    //            //.InWidthRate_Lambda([this]() { return 1.0f / ZoomRate.Get(); })
    //            .SecondToWidthRatio(10)
    //            .OnAddKey_Lambda([this](FShowKey* Key) {})
    //            .OnRemoveKey_Lambda([this](FShowKey* Key) {})
    //            .OnClickedKey_Lambda([this](FShowKey* Key) {})
    //            .OnChangedKey_Lambda([this](FShowKey* Key) {})
    //    ]

    //    + SVerticalBox::Slot()
    //    .Padding(2.0f)
    //    .AutoHeight()
    //    [
    //        SNew(SShowSequencerScrubPanel)
    //            .ShowSequencerEditorHelper(EditorHelper)
    //            .bDisplayAnimScrubBarEditing(true)
    //            .bAllowZoom(true)
    //            .OnUpdateZoom_Lambda([this](float InZoomRate)
    //                {
    //                    //ZoomRate.Set(InZoomRate);
    //                })
    //    ];
}

void SShowActionControllPanels::SelectAction(
    UActionBase* InAction, 
    TObjectPtr<UShowSequencer> InCastShow,
    TObjectPtr<UShowSequencer> InExecShow,
    TObjectPtr<UShowSequencer> InFinishShow)
{
    InShowSequencerEditorHelperMap.Empty();
    CrrAction = InAction;

    if (CastEditorHelper)
    {
        CastEditorHelper.Reset();
        CastEditorHelper = nullptr;
    }
    if (ExecEditorHelper)
    {
        ExecEditorHelper.Reset();
        ExecEditorHelper = nullptr;
    }
    if (FinishEditorHelper)
    {
        FinishEditorHelper.Reset();
        FinishEditorHelper = nullptr;
    }

    if (InCastShow)
    {
        CastEditorHelper = MakeShared<FShowSequencerEditorHelper>(InCastShow);
        InShowSequencerEditorHelperMap.Add("Cast", CastEditorHelper);
    }

    if (InExecShow)
    {
        ExecEditorHelper = MakeShared<FShowSequencerEditorHelper>(InExecShow);
        InShowSequencerEditorHelperMap.Add("Exec", CastEditorHelper);
    }

    if (InFinishShow)
    {
        FinishEditorHelper = MakeShared<FShowSequencerEditorHelper>(InFinishShow);
        InShowSequencerEditorHelperMap.Add("Finish", CastEditorHelper);
    }    

    ConstructRightWidget(Args);
    ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&InShowSequencerEditorHelperMap);
}
