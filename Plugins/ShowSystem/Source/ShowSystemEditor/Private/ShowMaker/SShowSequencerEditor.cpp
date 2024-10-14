// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerEditor.h"
#include "SlateOptMacros.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "SHorizontalResizableSplitter.h"
#include "ShowMaker/SShowSequencerControllPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerEditor::Construct(const FArguments& InArgs)
{
    EditorHelper = InArgs._EditorHelper;
    OnKeyDownSpace = InArgs._OnKeyDownSpace;

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
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            SNew(SBox)
                [
                    SNew(SButton)
                ]
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
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(2.0f)
        .FillHeight(1.0f)
        [
            SNew(SShowKeyBoxHandler)
                .ShowSequencerEditorHelper(EditorHelper)
                .Height(InArgs._Height)
                .MinWidth(InArgs._MinWidth)
                .InWidthRate_Lambda([this]() { return 1.0f / ZoomRate.Get(); })
                .SecondToWidthRatio(InArgs._SecondToWidthRatio)
                .OnAddKey(InArgs._OnAddKey)
                .OnRemoveKey(InArgs._OnRemoveKey)
                .OnClickedKey(InArgs._OnClickedKey)
                .OnChangedKey(InArgs._OnChangedKey)
        ]

        + SVerticalBox::Slot()
        .Padding(2.0f)
        .AutoHeight()
        [
            SNew(SShowSequencerScrubPanel)
                .ShowSequencerEditorHelper(EditorHelper)
                .bDisplayAnimScrubBarEditing(true)
                .bAllowZoom(true)
                .OnUpdateZoom_Lambda([this](float InZoomRate)
                    { 
                        ZoomRate.Set(InZoomRate);
                    })
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
