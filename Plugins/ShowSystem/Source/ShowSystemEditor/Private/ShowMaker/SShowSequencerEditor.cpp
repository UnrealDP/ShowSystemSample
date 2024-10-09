// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerEditor.h"
#include "SlateOptMacros.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "ShowMaker/SShowSequencerScrubPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerEditor::Construct(const FArguments& InArgs)
{
    EditorHelper = InArgs._EditorHelper;

	ChildSlot
	[
        SNew(SVerticalBox)
     
            + SVerticalBox::Slot()
            .Padding(2.0f)
            .FillHeight(1.0f)
            [
                SNew(SShowKeyBoxHandler)
                    .ShowSequencerEditorHelper(EditorHelper)
                    .Height(InArgs._Height)
                    .MinWidth(InArgs._MinWidth)
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
            ]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SShowSequencerEditor::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::SpaceBar)
    {
        // 스페이스바가 눌렸을 때 실행할 코드
        UE_LOG(LogTemp, Log, TEXT("Space bar pressed in SShowMakerWidget!"));

        // 원하는 작업 수행 후 FReply::Handled() 반환
        return FReply::Handled();
    }

    // 스페이스바가 아닌 경우 부모 클래스 처리로 전달
    return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}
