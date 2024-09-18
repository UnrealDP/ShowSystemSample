// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowMakerWidget.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowMakerWidget::Construct(const FArguments& InArgs)
{
    ShowSequencer = InArgs._ShowSequencer;

    // 기본 UI 레이아웃 구성
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Padding(5)
                [
                    SNew(STextBlock)
                        .Text(FText::FromString("ShowMaker Editor"))
                ]
                // 여기에 다른 위젯들을 추가해 편집 창을 완성
        ];

    if (ShowSequencer)
    {
        // ShowSequencer로부터 데이터를 받아 초기화
    }
    else
    {
        // 기본값으로 초기화
    }

    //ChildSlot
    //    [
    //        SNew(SVerticalBox)
    //            + SVerticalBox::Slot()
    //            .AutoHeight()
    //            [
    //                // 메뉴 바 추가
    //                FMenuBarBuilder::AddPullDownMenu(
    //                        FText::FromString("File"),
    //                        FText::FromString("Open file menu"),
    //                        FNewMenuDelegate::CreateSP(this, &SShowMakerWidget::MakeFileMenu)
    //                    )
    //            ]

    //            + SVerticalBox::Slot()
    //            .AutoHeight()
    //            [
    //                // 툴바 추가
    //                FToolBarBuilder::MakeToolBar()
    //                    .AddToolBarButton(
    //                        FUIAction(FExecuteAction::CreateSP(this, &SShowMakerWidget::OnButtonClick)),
    //                        NAME_None,
    //                        FText::FromString("Save"),
    //                        FText::FromString("Save the current show"),
    //                        FSlateIcon(FCoreStyle::Get().GetStyleSetName(), "Icons.Save")
    //                    )
    //            ]

    //            + SVerticalBox::Slot()
    //            [
    //                // 실제 위젯 콘텐츠
    //                SNew(STextBlock)
    //                    .Text(FText::FromString("Show Maker Content Goes Here"))
    //            ]
    //    ];
}

void SShowMakerWidget::MakeFileMenu(FMenuBuilder& MenuBuilder)
{
    MenuBuilder.AddMenuEntry(
        FText::FromString("Open"),
        FText::FromString("Open a file"),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(this, &SShowMakerWidget::OnOpenFile))
    );
}

void SShowMakerWidget::OnOpenFile()
{
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
