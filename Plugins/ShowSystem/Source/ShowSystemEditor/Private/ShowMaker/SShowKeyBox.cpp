// Fill out your copyright notice in the Description page of Project Settings.

#include "ShowMaker/SShowKeyBox.h"
#include "SlateOptMacros.h"
#include "Math/UnrealMathUtility.h"
#include "Fonts/FontMeasure.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyBox::Construct(const FArguments& InArgs)
{
	ShowKey = InArgs._ShowKey;
    Height = InArgs._Height;
    MinWidth = InArgs._MinWidth;
    SecondToWidthRatio = InArgs._SecondToWidthRatio;
    OnClick = InArgs._OnClick;
    OnChanged = InArgs._OnChanged;    

    ChildSlot
        [
            // 마우스 클릭 이벤트를 받기 위한 더미 위젯임
            SNew(SBox)
                .HAlign(HAlign_Left)
                .WidthOverride(0)
                .HeightOverride(Height.Get())
                .Visibility(EVisibility::Visible)
                [
                    SNew(SBorder)
                        .OnMouseButtonDown(this, &SShowKeyBox::OnMouseButtonDown)
                        .HAlign(HAlign_Left)
                        .VAlign(VAlign_Fill)
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

int32 SShowKeyBox::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    if (ShowKey)
    {
        const float StartX = ShowKey->StartTime * SecondToWidthRatio.Get();
        const float KeyWidth = FMath::Max(MinWidth.Get(), ShowKey->Length * SecondToWidthRatio.Get());

        // 클릭 영역을 저장
        ClickableBox = FBox2D(FVector2D(StartX, 0), FVector2D(StartX + KeyWidth, Height.Get()));

        // 박스 그리기
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, Height.Get()), FSlateLayoutTransform(FVector2D(StartX, 0))),
            FCoreStyle::Get().GetBrush("WhiteBrush"),
            ESlateDrawEffect::None,
            FLinearColor::Gray
        );

        // 텍스트 높이 측정
        TSharedRef<FSlateFontMeasure> FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        FVector2D TextSize = FontMeasureService->Measure(FText::FromString("TEMP_KEY_NAME"), FCoreStyle::Get().GetFontStyle("Regular"));

        // 박스의 중앙에 텍스트 배치
        float TextPosY = (Height.Get() - TextSize.Y) * 0.5f;
        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId + 1,
            AllottedGeometry.ToPaintGeometry(FVector2D(KeyWidth, TextSize.Y), FSlateLayoutTransform(FVector2D(StartX + 5, TextPosY))),
            "TEMP_KEY_NAME",
            FCoreStyle::Get().GetFontStyle("Regular"),
            ESlateDrawEffect::None,
            FLinearColor::Red
        );
    }

    return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
//    return LayerId;
}

FReply SShowKeyBox::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // 클릭 위치를 가져옴
    FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
    // 저장한 클릭 영역과 비교
    if (!ClickableBox.IsInside(LocalMousePosition))
    {
        return FReply::Handled();
    }

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (OnClick.IsBound() && ShowKey)
        {
            OnClick.Execute(ShowKey); // 클릭 시 키 정보 전달

            DragStartPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
            return FReply::Handled().CaptureMouse(SharedThis(this));
        }
    }

    return FReply::Unhandled();
}

FReply SShowKeyBox::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (HasMouseCapture())
    {
        DragStartPosition = FVector2D::ZeroVector;
        return FReply::Handled().ReleaseMouseCapture();
    }

    return FReply::Unhandled();
}

FReply SShowKeyBox::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (DragStartPosition == FVector2D::ZeroVector)
    {
		return FReply::Unhandled();
	}

    FVector2D DragCurrentPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
    float DragDeltaX = DragCurrentPosition.X - DragStartPosition.X;
    UE_LOG(LogTemp, Warning, TEXT("DragDeltaX: %f"), DragDeltaX);

    float DeltaTime = DragDeltaX / SecondToWidthRatio.Get();
    float StartTime = ShowKey->StartTime + DeltaTime;
    StartTime = FMath::Max(StartTime, 0.0f);
    ShowKey->StartTime = StartTime;

    DragStartPosition = DragCurrentPosition;

    if (OnChanged.IsBound())
	{
		OnChanged.Execute(ShowKey);
	}

    return FReply::Handled();
}