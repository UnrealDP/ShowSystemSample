// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowKeyBoxHandler.h"
#include "SlateOptMacros.h"
#include "SlateEditorUtils.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyBoxHandler::Construct(const FArguments& InArgs)
{
    ShowKeys = InArgs._ShowKeys;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    SecondToWidthRatio = InArgs._SecondToWidthRatio;
    OnAddKey = InArgs._OnAddKey;
    OnRemoveKey = InArgs._OnRemoveKey;
    OnClickedKey = InArgs._OnClickedKey;
    OnChangedKey = InArgs._OnChangedKey;

    ShowKeyBoxes.Empty();
    TSharedPtr<SVerticalBox> VerticalBox;

    ChildSlot
        [
            SAssignNew(VerticalBox, SVerticalBox)
            +SVerticalBox::Slot()
                .AutoHeight()
                [
                    CreateMenuBar()
                ]
                SLATE_SPACE_SLOT(0, 5)
        ];


    for (FShowKey* Key : ShowKeys)
    {
        TSharedPtr<SShowKeyBox> NewKeyBox;
        SAssignNew(NewKeyBox, SShowKeyBox)
            .ShowKey(Key)
            .Height(Height)
            .MinWidth(MinWidth)
            .SecondToWidthRatio(SecondToWidthRatio)
            .OnClick(this, &SShowKeyBoxHandler::OnKeyClicked)
            .OnChanged(OnChangedKey);

        ShowKeyBoxes.Add(NewKeyBox);

        VerticalBox->AddSlot()
            .AutoHeight()
            [
                NewKeyBox.ToSharedRef()
            ];
    }
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SShowKeyBoxHandler::CreateMenuBar()
{
    FMenuBarBuilder MenuBarBuilder(nullptr);

    // "Select Skeletal Mesh" 풀다운 메뉴 추가
    MenuBarBuilder.AddPullDownMenu(
        FText::FromString("Add Key"),
        FText::FromString("Add Key"),
        FNewMenuDelegate::CreateRaw(this, &SShowKeyBoxHandler::GenerateMenu)
    );

    return MenuBarBuilder.MakeWidget();
}

void SShowKeyBoxHandler::GenerateMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString("Key Type 1"),
        FText::FromString("Select Key Type 1"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &SShowKeyBoxHandler::OnKeyTypeSelected, FString("Key Type 1")))
    );

    MenuBuilder.AddMenuEntry(
        FText::FromString("Key Type 2"),
        FText::FromString("Select Key Type 2"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &SShowKeyBoxHandler::OnKeyTypeSelected, FString("Key Type 2")))
    );
}

void SShowKeyBoxHandler::OnKeyTypeSelected(FString SelectedKeyType)
{
    // 선택된 키 타입을 외부로 전달
    if (OnAddKey.IsBound())
    {
        //OnAddKey.Execute(SelectedKeyType);
    }

    if (MenuWindow.IsValid())
    {
        FSlateApplication::Get().DismissMenu(MenuWindow.ToSharedRef());
        MenuWindow = nullptr;
    }
}

// Key를 클릭했을 때 호출
void SShowKeyBoxHandler::OnKeyClicked(FShowKey* ClickedKey)
{
    // 키 관련 로직 처리
    if (OnClickedKey.IsBound())
	{
		OnClickedKey.Execute(ClickedKey);
	}
}