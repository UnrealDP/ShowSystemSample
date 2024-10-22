// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerEditor.h"
#include "SlateOptMacros.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "SHorizontalResizableSplitter.h"
#include "ShowMaker/SShowSequencerControllPanel.h"
#include "ShowMaker/SShowSequencerEditHeader.h"
#include "ShowMaker/SShowSequencerScrubBoard.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerEditor::Construct(const FArguments& InArgs)
{
    EditorHelper = InArgs._EditorHelper;
    OnKeyDownSpace = InArgs._OnKeyDownSpace;
    OnAddKey = InArgs._OnAddKey;
    OnRemoveKey = InArgs._OnRemoveKey;
    OnClickedKey = InArgs._OnClickedKey;

    ShowSequencerEditorHelperSortMap.Add("Show", EditorHelper);

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
							ConstructRightWidget(InArgs)
                        }
                        )
                    .InitialRatios(
                        { 0.1f, 0.9f }
                    )
            ]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SShowSequencerEditor::ConstructLeftWidget(const FArguments& InArgs)
{
    ShowSequencerEditHeader = SNew(SShowSequencerEditHeader)
        .TitleHeight(30)
        .Height(20)
        .Width(100)
        .OnAddShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBasePtr)
            {
                if (OnAddKey.IsBound())
                {
                    OnAddKey.Execute(ShowBasePtr);
                }
                IsUpdateKey = true;
            })
        .OnRemoveShowKeyEvent_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper)
            {
                if (OnRemoveKey.IsBound())
                {
                    OnRemoveKey.Execute();
                }
                IsUpdateKey = true;
            })
        .OnShowKeyClicked_Lambda([this](TSharedPtr<FShowSequencerEditorHelper> EditorHelper, UShowBase* ShowBasePtr)
            {
                if (OnClickedKey.IsBound())
                {
                    OnClickedKey.Execute(ShowBasePtr);
                }
            })
        .IsShowKeySelected(InArgs._IsShowKeySelected);

    ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&ShowSequencerEditorHelperSortMap);

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            ShowSequencerEditHeader.ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .AutoHeight()
        [
            SNew(SShowSequencerControllPanel)
                .ShowSequencerState(InArgs._ShowSequencerState)
                .OnPlay_Lambda([this]()
					{
						EditorHelper->Play();
					})
        ];
}

TSharedRef<SWidget> SShowSequencerEditor::ConstructRightWidget(const FArguments& InArgs)
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [
            SNew(SShowSequencerScrubBoard)
                .Height(30.0f)
                .TotalValue_Lambda([this]() { return EditorHelper->GetWidgetLengthAlignedToInterval(2.0f); })
                .CrrValue_Lambda([this]() { return EditorHelper->EditShowSequencerPtr->GetPassedTime(); })
                .OnValueChanged_Lambda([this](float InValue)
                    {
                        //EditorHelper->ScrubToSeconds(InValue);
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
                        .Height(InArgs._Height)
                        .MinWidth(InArgs._MinWidth)
                        .OnClickedKey(InArgs._OnClickedKey)
                        .OnChangedKey(InArgs._OnChangedKey)
                        .IsShowKeySelected(InArgs._IsShowKeySelected)
                ]
        ];
}

FReply SShowSequencerEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::SpaceBar)
    {
        if (OnKeyDownSpace.IsBound())
		{
			OnKeyDownSpace.Execute();
		}
        return FReply::Handled();
    }

    // 스페이스바가 아닌 경우 부모 클래스 처리로 전달
    return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SShowSequencerEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
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
            ShowSequencerEditHeader->RefreshShowKeyHeaderBoxs(&ShowSequencerEditorHelperSortMap);
        }
        IsUpdateKey = false;
    }
}