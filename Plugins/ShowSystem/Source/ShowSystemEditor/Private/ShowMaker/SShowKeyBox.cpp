// Fill out your copyright notice in the Description page of Project Settings.

#include "ShowMaker/SShowKeyBox.h"
#include "SlateOptMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Fonts/FontMeasure.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyBox::Construct(const FArguments& InArgs)
{
    ShowBase = InArgs._ShowBase;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    InWidthRate = InArgs._InWidthRate;
    SecondToWidthRatio = InArgs._SecondToWidthRatio;
    OnClick = InArgs._OnClick;
    OnChangedStartTime = InArgs._OnChangedStartTime;
    IsShowKeySelected = InArgs._IsShowKeySelected;

    ChildSlot
        [
            SNew(SOverlay)
                + SOverlay::Slot()
                .Padding(TAttribute<FMargin>::Create(TAttribute<FMargin>::FGetter::CreateLambda([this]() 
                    {
                        return FMargin(ClickableBox.Left, 0.0f, 0.0f, 0.0f);
                    })))
                [
                    SNew(SBox)
                        .WidthOverride_Lambda([this]() { return ClickableBox.GetSize().X; })
                        .HeightOverride_Lambda([this]() { return ClickableBox.GetSize().Y; })
                        [
                            SNullWidget::NullWidget
                            /*SNew(SBorder)
                                .HAlign(HAlign_Fill)
                                .VAlign(VAlign_Fill)*/
                        ]
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SShowKeyBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (ShowBase)
    {
        const float StartX = ShowBase->GetStartTime() * SecondToWidthRatio.Get();
        const float KeyWidth = FMath::Max(MinWidth.Get(), ShowBase->GetShowLength() * SecondToWidthRatio.Get()) * InWidthRate.Get();

        // 클릭 영역을 저장
        ClickableBox = FSlateRect(StartX, 0, StartX + KeyWidth, Height.Get());

        FLinearColor BoxColor = FLinearColor::Gray;
        FLinearColor TextColor = FLinearColor::Black;
        if (IsShowKeySelected.IsBound())
        {
            if (IsShowKeySelected.Execute(ShowBase))
            {
                BoxColor = FLinearColor::Blue;
                TextColor = FLinearColor::Red;
            }
        }
        // 박스 그리기
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, Height.Get()), FSlateLayoutTransform(FVector2D(StartX, 0))),
            FCoreStyle::Get().GetBrush("WhiteBrush"),
            ESlateDrawEffect::None,
            BoxColor
        );


        // 폰트 설정
        FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("Regular");
        FontInfo.Size = 12;
        FontInfo.TypefaceFontName = FName("Bold");

        // 텍스트 높이 측정
        TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        FVector2D TextSize = FontMeasureService->Measure(FText::FromString("TEMP_KEY_NAME"), FontInfo);

        // 박스의 중앙에 텍스트 배치
        float TextPosY = (Height.Get() - TextSize.Y) * 0.5f;
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, TextSize.Y), FSlateLayoutTransform(FVector2D(StartX + 5, TextPosY))),
            ShowBase->GetTitle(),
            FontInfo,
            ESlateDrawEffect::None,
            TextColor
        );
    }

    return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
//    return LayerId;
}

FReply SShowKeyBox::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FVector2D LocalClickPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

        if (ClickableBox.ContainsPoint(LocalClickPos))
        {
            if (OnClick.IsBound() && ShowBase)
            {
                OnClick.Execute(ShowBase);
            }

            DragStartPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
            bIsDragging = true;
            return FReply::Handled().CaptureMouse(SharedThis(this));
        }
    }
	return FReply::Unhandled();
}

FReply SShowKeyBox::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging)
    {
        FVector2D DragCurrentPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
        float DragDeltaX = DragCurrentPosition.X - DragStartPosition.X;

        float DeltaTime = DragDeltaX / SecondToWidthRatio.Get();
        float StartTime = ShowBase->GetStartTime() + DeltaTime;
        StartTime = FMath::Max(StartTime, 0.0f);

        DragStartPosition = DragCurrentPosition;

        if (OnChangedStartTime.IsBound())
        {
            OnChangedStartTime.Execute(ShowBase, StartTime);
        }

        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SShowKeyBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}
