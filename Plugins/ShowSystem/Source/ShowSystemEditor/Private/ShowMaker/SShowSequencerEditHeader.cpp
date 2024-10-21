// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerEditHeader.h"
#include "SlateOptMacros.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "SlateEditorUtils.h"
#include "SPositiveActionButton.h"
#include "RunTime/ShowSystem.h"
#include "RunTime/EShowKeyType.h"

#include "RunTime/ShowKeys/ShowAnimStatic.h"

#define LOCTEXT_NAMESPACE "SShowSequencerEditHeader"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerEditHeader::Construct(const FArguments& InArgs)
{
    TitleHeight = InArgs._TitleHeight;
    Height = InArgs._Height;
    Width = InArgs._Width;
    OnAddShowKeyEvent = InArgs._OnAddShowKeyEvent;
    OnRemoveShowKeyEvent = InArgs._OnRemoveShowKeyEvent;
    
    if (const UEnum* EnumPtr = FindObject<UEnum>(nullptr, TEXT("/Script/ShowSystem.EShowKeyType")))
    {
        // 마지막 인덱스는 언리얼에서 추가하는 _MAX 값이므로 제외
        int32 NumEnums = EnumPtr->NumEnums() - 1;
        for (int32 i = 0; i < NumEnums; ++i)
        {
            int32 EnumValue = EnumPtr->GetValueByIndex(i);
            EShowKeyType KeyType = static_cast<EShowKeyType>(EnumValue);

            if (KeyType == EShowKeyType::Max)
            {
                continue;
            }

            UScriptStruct* ScriptStruct = ShowSystem::GetShowKeyStaticStruct(KeyType);
            if (!ScriptStruct)
            {
				continue;
			}

            FString StructName = ScriptStruct->GetName();
            bool bContains = KeyOptions.ContainsByPredicate([StructName](const TSharedPtr<FString>& Option)
                {
                    return Option.IsValid() && Option->Equals(StructName);
                });
            if (bContains)
            {
				continue;
			}

            KeyOptions.Add(MakeShared<FString>(StructName));
        }
    }

    ChildSlot
        [
            SAssignNew(VerticalBox, SVerticalBox)
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SShowSequencerEditHeader::RefreshShowKeyHeaderBoxs(TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr)
{
    checkf(InShowSequencerEditorHelperSortMapPtr, TEXT("SShowSequencerEditHeader::RefreshShowKeyHeaderBoxs InShowSequencerEditorHelperSortMapPtr is nullptr."));

    ShowSequencerEditorHelperSortMapPtr = InShowSequencerEditorHelperSortMapPtr;

    VerticalBox->ClearChildren();

    // 'Add' 버튼 다시 추가
    VerticalBox->AddSlot()
        .AutoHeight()
        .HAlign(HAlign_Left)
        [
            SNew(SBox)
                .HeightOverride(TitleHeight.Get())
                [
                    SNew(SPositiveActionButton)
                        .Text(LOCTEXT("AddButton", "Add"))
                        .OnGetMenuContent(this, &SShowSequencerEditHeader::CreateAddKeyMenu)
                        .ToolTipText(LOCTEXT("AddButtonTooltip", "Add Show Key."))
                        .Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
                ]
        ];

    VerticalBox->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		[
            SNew(SSpacer).Size(FVector2D(0.0f, 3.0f))
		];

    // ShowSequencerEditorHelper의 ShowKeys 배열을 통해 새로운 위젯 생성
    for (auto& Elem : *ShowSequencerEditorHelperSortMapPtr)
    {
        FString Key = Elem.Key;
        TSharedPtr<FShowSequencerEditorHelper> Value = Elem.Value;

        VerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
                ConstructShowSequencerHeaderWidget(Value)
                /*SNew(SBox)
                    .HeightOverride(Height.Get())
                    [
                        ConstructShowSequencerHeaderWidget(Value)
                    ]*/
                
                /*SNew(SOverlay)
                    + SOverlay::Slot()
                    [
                        SNew(SBorder)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Fill)
                            .BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
                    ]
                    + SOverlay::Slot()
                    [
                        ConstructShowSequencerHeaderWidget(Value)
                    ]*/
			];
    }
}

TSharedRef<SWidget> SShowSequencerEditHeader::ConstructShowSequencerHeaderWidget(TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper)
{
    if(!ShowSequencerEditorHelper)
	{
		return SNullWidget::NullWidget;
	}

    TArray<UShowBase*>* RuntimeShowKeysPtr = ShowSequencerEditorHelper->RuntimeShowKeysPtr();
    if (!RuntimeShowKeysPtr)
    {
        return SNullWidget::NullWidget;
    }

    TSharedRef<SVerticalBox> ShowSequencerVerticalBox = SNew(SVerticalBox);
    for (UShowBase* ShowBasePtr : *RuntimeShowKeysPtr)
	{
        ShowSequencerVerticalBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			[
                SNew(SBox)
                    .HeightOverride(Height.Get())
                    [
                        SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .AutoWidth()
                            [
                                SNew(SButton)
                                    .HAlign(HAlign_Center)
                                    .VAlign(VAlign_Center)
                                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                                    .ToolTipText(LOCTEXT("RemoveButtonTooltip", "Remove Show Key."))
                                    .OnClicked_Lambda([this, ShowSequencerEditorHelper, ShowBasePtr]() -> FReply
                                        {
                                            return OnRemoveShowKey(ShowSequencerEditorHelper, ShowBasePtr);
                                        })
                                    [
                                        SNew(SImage)
                                            .Image(FAppStyle::Get().GetBrush("Icons.Minus"))
                                    ]
                            ]
                            + SHorizontalBox::Slot()
                            .HAlign(HAlign_Fill)
                            .AutoWidth()
                            [
                                SNew(STextBlock).Text(FText::FromString(ShowBasePtr->GetClass()->GetName()))
                            ]
                    ]
			];
	}

    return ShowSequencerVerticalBox;
}

TSharedRef<SWidget> SShowSequencerEditHeader::CreateAddKeyMenu()
{
    SelectedShowSequencerEditorHelper = nullptr;

    int Num = ShowSequencerEditorHelperSortMapPtr->Num();
    if (Num == 0)
    {
        return SNullWidget::NullWidget;
    }
    else if (Num == 1)
    {
        if (ShowSequencerEditorHelperSortMapPtr && ShowSequencerEditorHelperSortMapPtr->Num() > 0)
        {
            SelectedShowSequencerEditorHelper = *ShowSequencerEditorHelperSortMapPtr->FirstValue();
        }

        if (!SelectedShowSequencerEditorHelper)
        {
            return SNullWidget::NullWidget;
        }

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
                            .OnGenerateRow(this, &SShowSequencerEditHeader::GenerateKeyRow)
                            .OnSelectionChanged(this, &SShowSequencerEditHeader::OnAddKeySelected)
                    ]
            ];
    }
    else
    {
        return ShowSequencerMenuBuilder();
    }
}

TSharedRef<ITableRow> SShowSequencerEditHeader::GenerateKeyRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        .Padding(FMargin(10, 3, 5, 2))
        [
            SNew(STextBlock).Text(FText::FromString(*InItem))
        ];
}

TSharedRef<SWidget> SShowSequencerEditHeader::ShowSequencerMenuBuilder()
{
    FMenuBuilder MenuBuilder(true, nullptr);

    for (auto& Elem : *ShowSequencerEditorHelperSortMapPtr)
	{
        TSharedPtr<FString> OptionPtr = MakeShared<FString>(Elem.Key);

        MenuBuilder.AddMenuEntry(
            FText::FromString(Elem.Key),
            FText::FromString(Elem.Key),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateSP(this, &SShowSequencerEditHeader::OnAddSequencerKeySelected, OptionPtr)
            )
        );
	}

    return MenuBuilder.MakeWidget();
}

void SShowSequencerEditHeader::OnAddKeySelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo)
{
    OnAddKeySelected(SelectedItem);
}

void SShowSequencerEditHeader::OnAddKeySelected(TSharedPtr<FString> SelectedItem)
{
    if (!SelectedShowSequencerEditorHelper)
    {
        return;
    }

    if (SelectedItem.IsValid())
    {
        UScriptStruct* FoundStruct = FindObject<UScriptStruct>(nullptr, *FString::Printf(TEXT("/Script/ShowSystem.%s"), **SelectedItem), true);
        if (FoundStruct)
        {
            FInstancedStruct NewKey(FoundStruct);
            UShowBase* NewShowBasePtr = SelectedShowSequencerEditorHelper->AddKey(NewKey);

            if (NewShowBasePtr)
            {
                if (OnAddShowKeyEvent.IsBound())
                {
                    OnAddShowKeyEvent.Execute(SelectedShowSequencerEditorHelper, NewShowBasePtr);
                }
            }
        }
    }
}

void SShowSequencerEditHeader::OnAddSequencerKeySelected(TSharedPtr<FString> SelectedItem)
{
    SelectedShowSequencerEditorHelper = nullptr;
    for (auto& Elem : *ShowSequencerEditorHelperSortMapPtr)
    {
        if (Elem.Key == *SelectedItem)
        {
            SelectedShowSequencerEditorHelper = Elem.Value;
            break;
        }
    }

    FMenuBuilder SecondMenuBuilder(true, nullptr);

    for (TSharedPtr<FString> option : KeyOptions)
    {
        SecondMenuBuilder.AddMenuEntry(
			FText::FromString(*option),
			FText::FromString(*option),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SShowSequencerEditHeader::OnAddKeySelected, option)
			)
		);
    }

    // 새로운 메뉴를 현재 메뉴 위치에 표시
    FSlateApplication::Get().PushMenu(
        AsShared(),
        FWidgetPath(),
        SecondMenuBuilder.MakeWidget(),
        FSlateApplication::Get().GetCursorPos(),
        FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
    );
}


FReply SShowSequencerEditHeader::OnRemoveShowKey(TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper, UShowBase* ShowBasePtr)
{
    if (!ShowSequencerEditorHelper)
    {
        return FReply::Handled();
    }

    if (ShowSequencerEditorHelper->RemoveKey(ShowBasePtr))
    {
        if (OnRemoveShowKeyEvent.IsBound())
        {
            OnRemoveShowKeyEvent.Execute(ShowSequencerEditorHelper);
        }
    }

    return FReply::Handled();
}


#undef LOCTEXT_NAMESPACE