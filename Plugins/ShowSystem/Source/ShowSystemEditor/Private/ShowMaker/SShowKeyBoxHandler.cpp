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
    ShowSequencerEditorHelper = InArgs._ShowSequencerEditorHelper;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    InWidthRate = InArgs._InWidthRate;
    SecondToWidthRatio = InArgs._SecondToWidthRatio;
    OnAddKey = InArgs._OnAddKey;
    OnClickedKey = InArgs._OnClickedKey;
    OnChangedKey = InArgs._OnChangedKey;

    KeyOptions.Add(MakeShared<FString>("Add ShowAnimStatic"));
    KeyOptions.Add(MakeShared<FString>("Add Key 2"));

    ChildSlot
        [
            SAssignNew(VerticalBox, SVerticalBox)
        ];

    RefreshShowKeyWidgets();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SShowKeyBoxHandler::RefreshShowKeyWidgets()
{
    if (!ShowSequencerEditorHelper.IsValid() || !VerticalBox.IsValid())
    {
        return;
    }

    // 기존 위젯 제거
    VerticalBox->ClearChildren();

    // 'Add' 버튼 다시 추가
    /*VerticalBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Left)
        .Padding(3.0f, 3.0f)
        [
            SNew(SPositiveActionButton)
                .Text(LOCTEXT("AddButton", "Add"))
                .OnGetMenuContent(this, &SShowKeyBoxHandler::CreateAddKeyMenu)
                .ToolTipText(LOCTEXT("AddButtonTooltip", "Add Show Key."))
                .Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
        ];

    VerticalBox->AddSlot()
        .AutoHeight()
        [
            SNew(SSpacer).Size(FVector2D(0.0f, 5.0f))
        ];*/

    // ShowSequencerEditorHelper의 ShowKeys 배열을 통해 새로운 위젯 생성
    TArray<TObjectPtr<UShowBase>>* RuntimeShowKeysPtr = ShowSequencerEditorHelper->RuntimeShowKeysPtr();
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
                    .InWidthRate_Lambda([this]() { return InWidthRate.Get(); })
                    .SecondToWidthRatio(SecondToWidthRatio)
                    .OnClick(this, &SShowKeyBoxHandler::OnKeyClicked)
                    .OnChangedStartTime_Lambda([this](UShowBase* ShowBase, float StartTime) 
                        { 
                            ShowSequencerEditorHelper->SetShowBaseStartTime(ShowBase, StartTime); 

                            if (OnChangedKey.IsBound())
							{
								OnChangedKey.Execute(ShowBase);
							}
                        })
                    .IsShowKeySelected(this, &SShowKeyBoxHandler::IsShowKeySelected)
            ];
    }
}

TSharedRef<SWidget> SShowKeyBoxHandler::CreateAddKeyMenu()
{
    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("Menu.Background"))
        .Padding(2)
        [
            SNew(SBox)
                .HAlign(HAlign_Fill)
                [
                    SNew(SListView<TSharedPtr<FString>>)
                        .ItemHeight(24)
                        .ListItemsSource(&KeyOptions)
                        .OnGenerateRow(this, &SShowKeyBoxHandler::GenerateKeyRow)
                        .OnSelectionChanged(this, &SShowKeyBoxHandler::OnAddKeySelected)
                ]
        ];
}

TSharedRef<ITableRow> SShowKeyBoxHandler::GenerateKeyRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        .Padding(FMargin(10, 3, 5, 2))
        [
            SNew(STextBlock).Text(FText::FromString(*InItem))
        ];
}

void SShowKeyBoxHandler::OnAddKeySelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    if (!ShowSequencerEditorHelper.IsValid() || !ShowSequencerEditorHelper->EditShowSequencer)
	{
		return;
	}

    if (SelectedItem.IsValid())
    {
        TObjectPtr<UShowBase> NewShowBase = nullptr;
        if (*SelectedItem == "Add ShowAnimStatic")
        {
            NewShowBase = ShowSequencerEditorHelper->AddKey<FShowAnimStaticKey>();
        }
        else if (*SelectedItem == "Add Key 2")
        {
            // Add Key 2 선택 시 실행할 코드
        }

        if (NewShowBase)
        {
            if (OnAddKey.IsBound())
            {
                RefreshShowKeyWidgets();
                OnAddKey.Execute(NewShowBase);
            }
        }
    }
}

// Key를 클릭했을 때 호출
void SShowKeyBoxHandler::OnKeyClicked(UShowBase* ClickedhowBase)
{
    // 키 관련 로직 처리
    if (OnClickedKey.IsBound())
	{
        ShowSequencerEditorHelper->SelectedShowBase = ClickedhowBase;
		OnClickedKey.Execute(ClickedhowBase);
	}
}

bool SShowKeyBoxHandler::IsShowKeySelected(UShowBase* ShowBase)
{
    if (!ShowSequencerEditorHelper->SelectedShowBase)
    {
		return false;
	}
    return ShowSequencerEditorHelper->SelectedShowBase.Get() == ShowBase;
}

#undef LOCTEXT_NAMESPACE