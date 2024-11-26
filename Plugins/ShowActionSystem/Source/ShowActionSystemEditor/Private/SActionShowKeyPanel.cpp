// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionShowKeyPanel.h"
#include "SlateOptMacros.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "ShowMaker/SShowKeyBoxHandler.h"
#include "ShowMaker/SShowSequencerScrubBoard.h"
#include "ActionBase.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SActionShowKeyPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
        SNew(SOverlay)
            + SOverlay::Slot()
            [
                SNew(SShowSequencerScrubBoard)
                    .Height(30)
                    .TotalValue_Lambda([this]() 
                        { 
                            float TotalSkillDuration = 0.0f;
                            if (CrrActionPtr)
                            {
                                const FActionBaseData* ActionBaseData = CrrActionPtr->GetActionBaseData();
                                TotalSkillDuration += ActionBaseData->CastDuration;
                                TotalSkillDuration += ActionBaseData->ExecDuration;
                            }

                            float TotalShowDuration = 0.0f;
                            if (ShowSequencerEditorHelperSortMapPtr)
                            {
                                if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Cast"))
                                {
                                    TotalShowDuration = (*ShowSequencerEditorHelperSortMapPtr)["Cast"]->GetWidgetLengthAlignedToInterval(0.0f);
                                }
                                if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Exec"))
                                {
                                    float ExecShowDuration = (*ShowSequencerEditorHelperSortMapPtr)["Exec"]->GetWidgetLengthAlignedToInterval(0.0f);
                                    const FActionBaseData* ActionBaseData = CrrActionPtr->GetActionBaseData();
                                    ExecShowDuration += ActionBaseData->CastDuration;
                                    TotalShowDuration = FMath::Max(TotalShowDuration, ExecShowDuration);
                                    
                                }
                                if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Finish"))
                                {
                                    float FinishShowDuration = (*ShowSequencerEditorHelperSortMapPtr)["Finish"]->GetWidgetLengthAlignedToInterval(0.0f);
                                    const FActionBaseData* ActionBaseData = CrrActionPtr->GetActionBaseData();
                                    FinishShowDuration += ActionBaseData->CastDuration;
                                    FinishShowDuration += ActionBaseData->ExecDuration;
                                    TotalShowDuration = FMath::Max(TotalShowDuration, FinishShowDuration);
                                }
                            }

                            ScrubBoardTotalValue = FMath::Max(TotalSkillDuration, TotalShowDuration);
                            return ScrubBoardTotalValue;
                        })
                    .CrrValue_Lambda([this]()
                        { 
                            return CrrActionPtr ? CrrActionPtr->GetPassedTime() : 0.0f;
                        })
                    .OnValueChanged_Lambda([this](float InValue)
                        {
                            if (CrrActionPtr)
                            {
								CrrActionPtr->SetPassedTime(InValue);
                                CrrActionPtr->Pause();

                                if (GEditor && GEditor->GetActiveViewport())
                                {
                                    GEditor->GetActiveViewport()->InvalidateDisplay();
                                }
                            }
                        })
            ]
            + SOverlay::Slot()
            [
                SNew(SBox)
                    .Padding(FMargin(0, 30, 0, 0))
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Top)
                    [
                        SAssignNew(VerticalBox, SVerticalBox)
                    ]
            ]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActionShowKeyPanel::SetShowSequencerEditorHelperSortMap(
    const SShowActionControllPanels::FArguments& InArgs,
    UActionBase* InCrrActionPtr, 
    TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr)
{
	ShowSequencerEditorHelperSortMapPtr = InShowSequencerEditorHelperSortMapPtr;
    CrrActionPtr = InCrrActionPtr;

    VerticalBox->ClearChildren();

    if (ShowSequencerEditorHelperSortMapPtr)
    {
        if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Cast"))
        {
            ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Cast"], 0);
        }
        if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Exec"))
        {
            TAttribute<float> PaddigLeft = TAttribute<float>::Create(
                TAttribute<float>::FGetter::CreateLambda([&]()
                    {
                        if (CrrActionPtr)
                        {
                            const FActionBaseData* ActionBaseData = CrrActionPtr->GetActionBaseData();
                            return ActionBaseData->CastDuration;
                        }
                        return 0.0f;
                    })
            );

            ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Exec"], PaddigLeft);
        }
        if (ShowSequencerEditorHelperSortMapPtr->ContainsKey("Finish"))
        {
            TAttribute<float> PaddigLeft = TAttribute<float>::Create(
                TAttribute<float>::FGetter::CreateLambda([&]()
                    {
                        if (CrrActionPtr)
                        {
                            const FActionBaseData* ActionBaseData = CrrActionPtr->GetActionBaseData();
                            return ActionBaseData->CastDuration + ActionBaseData->ExecDuration;
                        }
                        return 0.0f;
                    })
            );

            ConstructShowSequencerWidget(InArgs, (*ShowSequencerEditorHelperSortMapPtr)["Finish"], PaddigLeft);
        }
    }
}

void SActionShowKeyPanel::ConstructShowSequencerWidget(
    const SShowActionControllPanels::FArguments& InArgs,
    TSharedPtr<FShowSequencerEditorHelper>& EditorHelper,
    TAttribute<float> PaddigLeft)
{
    const TArray<UShowBase*>* RuntimeShowKeysPtr = EditorHelper->RuntimeShowKeysPtr();
    for (UShowBase* ShowBasePtr : *RuntimeShowKeysPtr)
    {
        VerticalBox->AddSlot()
            .AutoHeight()
            .HAlign(HAlign_Fill)
            [
                SNew(SShowKeyBox)
                    .ShowBasePtr(ShowBasePtr)
                    .Height(20)
                    .MinWidth(30)
                    .PaddigLeft(PaddigLeft)
                    .SecondToWidthRatio_Lambda([this, EditorHelper]()
                        {
                            return WidgetWidth / ScrubBoardTotalValue;
                            //return WidgetWidth / EditorHelper->GetWidgetLengthAlignedToInterval(2.0f);
                        })
                    .OnClick_Lambda([this, InArgs, EditorHelper](UShowBase* ShowBasePtr)
						{
							if (InArgs._OnSelectedKey.IsBound())
							{
								InArgs._OnSelectedKey.Execute(EditorHelper, ShowBasePtr);
							}
						})
                    .OnChangedStartTime_Lambda([this, InArgs, EditorHelper](UShowBase* ChangedShowBasePtr, float StartTime)
                        {
                            EditorHelper->SetShowBaseStartTime(ChangedShowBasePtr, StartTime);

                            if (InArgs._OnSelectedKey.IsBound())
                            {
                                InArgs._OnSelectedKey.Execute(EditorHelper, ChangedShowBasePtr);
                            }
                        })
                    .IsShowKeySelected(InArgs._IsShowKeySelected)
            ];
    }
}

void SActionShowKeyPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    WidgetWidth = AllottedGeometry.GetLocalSize().X;
}