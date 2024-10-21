// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerScrubBoard.h"
#include "SlateOptMacros.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerScrubBoard::Construct(const FArguments& InArgs)
{
	Height = InArgs._Height;
	FPS = InArgs._FPS;
	TotalValue = InArgs._TotalValue;
	CrrValue = InArgs._CrrValue;
    OnValueChanged = InArgs._OnValueChanged;
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SShowSequencerScrubBoard::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const float Width = AllottedGeometry.GetLocalSize().X;
    const float HeightValue = Height.Get();
    const float TotalTime = TotalValue.Get();
    const float BarHeightValue = AllottedGeometry.GetLocalSize().Y;
    const int32 FrameRate = FPS.Get();
    const float CurrentValue = CrrValue.Get();
    const FLinearColor TimeLineBoxColor = FLinearColor::Black;
    const FLinearColor LineColor = FLinearColor::Gray;
    const FLinearColor MarkerColor = FLinearColor::Red;
    const float MarkerWidth = 10.0f;

    // 바깥쪽 외곽선 박스
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(FVector2D(Width, HeightValue), FSlateLayoutTransform(FVector2D(0, 0))),
        FCoreStyle::Get().GetBrush("WhiteBrush"),
        ESlateDrawEffect::None,
        FLinearColor::Gray
    );

    const float BorderThickness = 1.0f; // 외곽선 두께
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId + 1,
        AllottedGeometry.ToPaintGeometry(FVector2D(Width - 2 * BorderThickness, HeightValue - 2 * BorderThickness), FSlateLayoutTransform(FVector2D(BorderThickness, BorderThickness))),
        FCoreStyle::Get().GetBrush("WhiteBrush"),
        ESlateDrawEffect::None,
        TimeLineBoxColor
    );

    // Time markers (6.2, 6.3)
    float MarkerInterval = (FrameRate == 0) ? 1.0f : 1.0f / FrameRate;
    float MinorMarkerInterval = (FrameRate == 0) ? 0.1f : 0.5f;

    int32 Index = 0;
    int32 LastIndex = FMath::CeilToInt(TotalTime / MinorMarkerInterval); // 전체 루프에서의 마지막 인덱스 계산
    const float Tolerance = 0.001f;  // 부동소수점 오차 허용 범위

    for (float Time = 0; Time <= TotalTime + Tolerance; Time += MinorMarkerInterval, ++Index)
    {
        float XPos = (Time / TotalTime) * Width;

        // 첫 번째 라인만 초록색으로 설정
        FLinearColor CurrentLineColor = (Index == 0) ? FLinearColor::Green : LineColor;

        // 인덱스를 사용하여 메인선과 서브선을 구분
        bool bIsMainMarker = (Index % FMath::RoundToInt(MarkerInterval / MinorMarkerInterval)) == 0;
        float LineHeight = bIsMainMarker ? BarHeightValue : HeightValue * 0.4f;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId + 2,
            AllottedGeometry.ToPaintGeometry(FVector2D(1, LineHeight), FSlateLayoutTransform(FVector2D(XPos, 0))),
            FCoreStyle::Get().GetBrush("WhiteBrush"),
            ESlateDrawEffect::None,
            CurrentLineColor
        );

        // 메인선 위치에 시간/프레임 텍스트 추가
        if (bIsMainMarker)
        {
            FString TimeText = FrameRate == 0 ? FString::Printf(TEXT("%.0fs"), Time) : FString::Printf(TEXT("%.0ff"), Time * FrameRate);
            FSlateFontInfo FontInfo = FCoreStyle::GetDefaultFontStyle("Regular", 10);
            FVector2D TextSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(TimeText, FontInfo);

            // 마지막 반복일 때 텍스트를 좌측에 배치
            FVector2D TextPosition;
            if (FMath::IsNearlyEqual(Time, TotalTime, Tolerance))
            {
                TextPosition = FVector2D(XPos - TextSize.X - 3, HeightValue - TextSize.Y - 2); // 마지막 텍스트는 좌측으로
            }
            else
            {
                TextPosition = FVector2D(XPos + 3, HeightValue - TextSize.Y - 2); // 나머지는 우측에 배치
            }

            FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId + 3,
                AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPosition)),
                TimeText,
                FontInfo,
                ESlateDrawEffect::None,
                LineColor
            );
        }
    }

    // 마커 위치 계산 및 영역 저장
    float CurrentX = (CurrentValue / TotalTime) * Width - MarkerWidth / 2.0f;
    MarkerBounds = FSlateRect(CurrentX, 0.0f, CurrentX + MarkerWidth, HeightValue);

    // 현재 위치 마커 그리기
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId + 2,
        AllottedGeometry.ToPaintGeometry(FVector2D(MarkerWidth, HeightValue), FSlateLayoutTransform(FVector2D(CurrentX, 0))),
        FCoreStyle::Get().GetBrush("WhiteBrush"),
        ESlateDrawEffect::None,
        MarkerColor
    );

    // 현재 위치 마커 연장선
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId + 3,
        AllottedGeometry.ToPaintGeometry(FVector2D(1, BarHeightValue), FSlateLayoutTransform(FVector2D(CurrentX + MarkerWidth / 2.0f, 0))), // Marker 중앙에 선이 오도록 설정
        FCoreStyle::Get().GetBrush("WhiteBrush"),
        ESlateDrawEffect::None,
        MarkerColor
    );

    return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 4, InWidgetStyle, bParentEnabled);
}

FReply SShowSequencerScrubBoard::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        FVector2D LocalClickPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());

        if (MarkerBounds.ContainsPoint(LocalClickPos))
        {
            bIsDragging = true;
            return FReply::Handled().CaptureMouse(SharedThis(this));
        }
    }
    return FReply::Unhandled();
}

FReply SShowSequencerScrubBoard::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging)
    {
        const float Width = MyGeometry.GetLocalSize().X;
        const float MouseX = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X;
        const float NewValue = FMath::Clamp((MouseX / Width) * TotalValue.Get(), 0.0f, TotalValue.Get());

        OnValueChanged.ExecuteIfBound(NewValue);

        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SShowSequencerScrubBoard::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = false;
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}