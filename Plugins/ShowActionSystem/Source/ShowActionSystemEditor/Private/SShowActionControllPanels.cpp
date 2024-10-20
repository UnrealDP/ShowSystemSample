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
                                if (OnAddKey.IsBound())
                                {
                                    OnAddKey.Execute(EditorHelper, ShowBase);
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
                .TotalValue_Lambda([EditorHelper]() { return EditorHelper->GetWidgetLengthAlignedToInterval(2.0f); })
                .CrrValue_Lambda([EditorHelper]() { return EditorHelper->EditShowSequencer->GetPassedTime(); })
                .OnValueChanged_Lambda([EditorHelper](float InValue)
					{
						//EditorHelper->EditShowSequencer->SetPassedTime(InValue);
					})
        ]
        + SOverlay::Slot()
        [
            SNew(SBox)
                .Padding(FMargin(0, 30, 0, 0))
                .HAlign(HAlign_Left)
                .VAlign(VAlign_Top)
                [
                    SAssignNew(ShowKeyBoxHandler, SShowKeyBoxHandler)
                        .EditorHelper(EditorHelper)
                        .Height(30)
                        .MinWidth(100)
                        .OnClickedKey_Lambda([this, EditorHelper](UShowBase* ShowBase)
                            {
                                if (OnSelectedKey.IsBound())
								{
                                    OnSelectedKey.Execute(EditorHelper, ShowBase);
								}
                            })
                        .OnChangedKey_Lambda([EditorHelper](UShowBase* ShowBase)
                            {
                                EditorHelper->EditShowSequencer->MarkPackageDirty();
                            })
                ]
        ];
}

void SShowActionControllPanels::SelectAction(
    UActionBase* InAction, 
    TObjectPtr<UShowSequencer> InCastShow,
    TObjectPtr<UShowSequencer> InExecShow,
    TObjectPtr<UShowSequencer> InFinishShow)
{
    /*ShowSequencerEditorHelperMap.Empty();
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
        CastEditorHelper = MakeShared<FShowSequencerEditorHelper>();
        CastEditorHelper->EditShowSequencer = InCastShow;
        ShowSequencerEditorHelperMap.Add("Cast", CastEditorHelper);
    }

    if (InExecShow)
    {
        ExecEditorHelper = MakeShared<FShowSequencerEditorHelper>();
        ExecEditorHelper->EditShowSequencer = InExecShow;
        ShowSequencerEditorHelperMap.Add("Exec", ExecEditorHelper);
    }

    if (InFinishShow)
    {
        FinishEditorHelper = MakeShared<FShowSequencerEditorHelper>();
        FinishEditorHelper->EditShowSequencer = InFinishShow;
        ShowSequencerEditorHelperMap.Add("Finish", FinishEditorHelper);
    }    

    ConstructRightWidget(Args);
    ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&ShowSequencerEditorHelperMap);*/
}

void SShowActionControllPanels::ChangeShow(EActionState ActionState, TObjectPtr<UShowSequencer> NewShow)
{
    switch (ActionState)
    {
        case EActionState::Cast:
	    	if (CastEditorHelper)
	    	{
                if (TSharedPtr<FShowSequencerEditorHelper>* ExistingHelper = ShowSequencerEditorHelperMap.Find("Cast"))
                {
                    // "Cast" 키가 존재하면 해당 객체의 Show 변수를 변경
                    (*ExistingHelper)->EditShowSequencer = NewShow;
                }
	    	}
            else
            {
                CastEditorHelper = MakeShared<FShowSequencerEditorHelper>();
                CastEditorHelper->EditShowSequencer = NewShow;
                ShowSequencerEditorHelperMap.Add("Cast", CastEditorHelper);
            }
	    	break;
        default:
            break;
    }

    ConstructRightWidget(Args);
    ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&ShowSequencerEditorHelperMap);
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
            ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&ShowSequencerEditorHelperMap);
        }
        IsUpdateKey = false;
    }
}