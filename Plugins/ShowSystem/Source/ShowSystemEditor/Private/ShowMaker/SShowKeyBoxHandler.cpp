// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowKeyBoxHandler.h"
#include "SlateOptMacros.h"
#include "SlateEditorUtils.h"
#include "SPositiveActionButton.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "InstancedStruct.h"

#include "RunTime/ShowKeys/ShowAnimStatic.h"

#define LOCTEXT_NAMESPACE "SShowKeyBoxHandler"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyBoxHandler::Construct(const FArguments& InArgs)
{
    Args = InArgs;
    EditorHelper = InArgs._EditorHelper;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    OnClickedKey = InArgs._OnClickedKey;
    OnChangedStartTime = InArgs._OnChangedStartTime;

    ChildSlot
        [
            SAssignNew(VerticalBox, SVerticalBox)
        ];

    RefreshShowKeyWidgets();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SShowKeyBoxHandler::RefreshShowKeyWidgets()
{
    if (!EditorHelper.IsValid() || !VerticalBox.IsValid())
    {
        return;
    }

    // 기존 위젯 제거
    VerticalBox->ClearChildren();

    // EditorHelper의 ShowKeys 배열을 통해 새로운 위젯 생성
    const TArray<UShowBase*>* RuntimeShowKeysPtr = EditorHelper->RuntimeShowKeysPtr();
    for (UShowBase* ShowBasePtr : *RuntimeShowKeysPtr)
    {
        VerticalBox->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Fill)
            [
                SNew(SShowKeyBox)
                    .ShowBasePtr(ShowBasePtr)
                    .Height(Height)
                    .MinWidth(MinWidth)
                    .SecondToWidthRatio_Lambda([this]() 
                        { 
                            return WidgetWidth / EditorHelper->GetWidgetLengthAlignedToInterval(2.0f);
                        })
                    .OnClick(Args._OnClickedKey)
                    .OnChangedStartTime_Lambda([this](UShowBase* ChangedShowBasePtr, float StartTime) 
                        { 
                            EditorHelper->SetShowBaseStartTime(ChangedShowBasePtr, StartTime);

                            if (OnChangedStartTime.IsBound())
							{
                                OnChangedStartTime.Execute(ChangedShowBasePtr);
							}
                        })
                    .IsShowKeySelected(Args._IsShowKeySelected)
            ];
    }
}

void SShowKeyBoxHandler::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    WidgetWidth = AllottedGeometry.GetLocalSize().X;
}

#undef LOCTEXT_NAMESPACE