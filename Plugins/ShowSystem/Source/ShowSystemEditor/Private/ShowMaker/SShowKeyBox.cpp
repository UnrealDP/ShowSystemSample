// Fill out your copyright notice in the Description page of Project Settings.

#include "ShowMaker/SShowKeyBox.h"
#include "SlateOptMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Fonts/FontMeasure.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyBox::Construct(const FArguments& InArgs)
{
    ShowBasePtr = InArgs._ShowBasePtr;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
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
                        ]
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SShowKeyBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (ShowBasePtr)
    {
        const float StartTime = ShowBasePtr->GetStartTime();
        const float ShowLength = ShowBasePtr->GetLength();
        const float StartX = StartTime * SecondToWidthRatio.Get();
        const float KeyWidth = FMath::Max(MinWidth.Get(), ShowLength * SecondToWidthRatio.Get());

        // 클릭 영역을 저장
        ClickableBox = FSlateRect(StartX, 0, StartX + KeyWidth, Height.Get());

        bool IsSelected = false;
        FLinearColor BoxColor = FLinearColor(0.15f, 0.15f, 0.15f);
        FLinearColor TextColor = FLinearColor(0.6f, 0.6f, 0.6f);
        if (IsShowKeySelected.IsBound())
        {
            IsSelected = IsShowKeySelected.Execute(ShowBasePtr);
        }
        if (IsSelected)
        {
            BoxColor = FLinearColor(0.0f, 0.0f, 0.5f);
            TextColor = FLinearColor(0.0f, 0.5f, 0.0f);
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
        FontInfo.Size = 10;
        FontInfo.TypefaceFontName = FName("Bold");

        // 텍스트 높이 측정
        TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        FVector2D TextSize = FontMeasureService->Measure(FText::FromString("TEMP_KEY_NAME"), FontInfo);

        // 박스의 중앙에 텍스트 배치
        float TextPosY = (Height.Get() - TextSize.Y) * 0.5f;

        if (!IsSelected)
        {
            // 텍스트 외곽선 두께 설정
            const float OutlineThickness = 1.5f;
            // 텍스트 외곽선을 그리기 (검정색)
//            FSlateDrawElement::MakeText(
//                OutDrawElements,
//                ++LayerId,
////                AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, TextSize.Y), FSlateLayoutTransform(FVector2D(StartX + 5 - OutlineThickness, TextPosY - OutlineThickness))),
//                AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, TextSize.Y), FSlateLayoutTransform(FVector2D(StartX + 5 + OutlineThickness, TextPosY + OutlineThickness))),
//                ShowBasePtr->GetTitle(),
//                FontInfo,
//                ESlateDrawEffect::None,
//                FLinearColor::Black
//            );
        }

        FSlateDrawElement::MakeText(
            OutDrawElements,
            ++LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, TextSize.Y), FSlateLayoutTransform(FVector2D(StartX + 5, TextPosY))),
            FString::Printf(TEXT("%s (%.2f / %.2f)"), *ShowBasePtr->GetTitle(), StartTime, ShowLength),
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
            if (OnClick.IsBound() && ShowBasePtr)
            {
                OnClick.Execute(ShowBasePtr);
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
        float StartTime = ShowBasePtr->GetStartTime() + DeltaTime;
        StartTime = FMath::Max(StartTime, 0.0f);

        DragStartPosition = DragCurrentPosition;

        if (OnChangedStartTime.IsBound())
        {
            OnChangedStartTime.Execute(ShowBasePtr, StartTime);
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
