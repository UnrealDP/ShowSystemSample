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
    EditorHelper = InArgs._EditorHelper;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    OnClickedKey = InArgs._OnClickedKey;
    OnChangedKey = InArgs._OnChangedKey;

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
    TArray<TObjectPtr<UShowBase>>* RuntimeShowKeysPtr = EditorHelper->RuntimeShowKeysPtr();
    for (TObjectPtr<UShowBase>& ShowBase : *RuntimeShowKeysPtr)
    {
        VerticalBox->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Fill)
            [
                SNew(SShowKeyBox)
                    .ShowBase(ShowBase)
                    .Height(Height)
                    .MinWidth(MinWidth)
                    .SecondToWidthRatio(TAttribute<float>::Create(TAttribute<float>::FGetter::CreateSP(this, &SShowKeyBoxHandler::GetSecondToWidthRatio)))
                    .OnClick(this, &SShowKeyBoxHandler::OnKeyClicked)
                    .OnChangedStartTime_Lambda([this](UShowBase* ShowBase, float StartTime) 
                        { 
                            EditorHelper->SetShowBaseStartTime(ShowBase, StartTime); 

                            if (OnChangedKey.IsBound())
							{
								OnChangedKey.Execute(ShowBase);
							}
                        })
                    .IsShowKeySelected(this, &SShowKeyBoxHandler::IsShowKeySelected)
            ];
    }
}

float SShowKeyBoxHandler::GetSecondToWidthRatio()
{
    return WidgetWidth / EditorHelper->GetWidgetLengthAlignedToInterval(2.0f);
}

void SShowKeyBoxHandler::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    WidgetWidth = AllottedGeometry.GetLocalSize().X;
}

// Key를 클릭했을 때 호출
void SShowKeyBoxHandler::OnKeyClicked(UShowBase* ClickedhowBase)
{
    // 키 관련 로직 처리
    if (OnClickedKey.IsBound())
	{
		OnClickedKey.Execute(ClickedhowBase);
	}
}

bool SShowKeyBoxHandler::IsShowKeySelected(UShowBase* ShowBase)
{
    if (!EditorHelper->SelectedShowBase)
    {
		return false;
	}
    return EditorHelper->SelectedShowBase == ShowBase;
}

#undef LOCTEXT_NAMESPACE