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
                SLATE_SPACE_SLOT(0, 10)
        ];


    for (FShowKey* Key : ShowKeys)
    {
        TSharedPtr<SShowKeyBox> NewKeyBox;
        SAssignNew(NewKeyBox, SShowKeyBox)
            .ShowKey(Key)
            .Height(Height)
            .MinWidth(MinWidth)
            .SecondToWidthRatio(SecondToWidthRatio)
            .OnClick(this, &SShowKeyBoxHandler::OnKeyClicked);

        ShowKeyBoxes.Add(NewKeyBox);

        VerticalBox->AddSlot()
            .AutoHeight()
            [
                NewKeyBox.ToSharedRef()
            ];
    }
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

//int32 SShowKeyBoxHandler::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
//    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
//{
//    for (int32 Index = 0; Index < ShowKeyBoxes.Num(); ++Index)
//    {
//        // 바 왼쪽 정보 출력 박스 (임시로 키 이름 출력)
//        FSlateDrawElement::MakeText(
//            OutDrawElements,
//            LayerId + 1,
//            AllottedGeometry.ToPaintGeometry(FVector2D(100, 20), FSlateLayoutTransform(FVector2D(0, Index * 30))),
//            FString::Printf(TEXT("Key %d"), Index),
//            FCoreStyle::Get().GetFontStyle("Regular"),
//            ESlateDrawEffect::None,
//            FLinearColor::White
//        );
//    }
//
//    return LayerId;
//}


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
}

// 우클릭 시 Remove Key
FReply SShowKeyBoxHandler::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && SelectedKey)
    {
        // 우클릭한 키 삭제
        if (OnRemoveKey.IsBound())
        {
            OnRemoveKey.Execute(SelectedKey);
        }
        return FReply::Handled();
    }
    return FReply::Unhandled();
}
