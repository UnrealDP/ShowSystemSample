// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnShowKeyClicked, FShowKey*);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIsShowKeySelected, FShowKey*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowKeyBox : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SShowKeyBox) {}
        SLATE_ARGUMENT(FShowKey*, ShowKey)
        SLATE_ATTRIBUTE(float, SecondToWidthRatio) // 1초당 width 비율
        SLATE_ATTRIBUTE(float, Height)
        SLATE_ATTRIBUTE(float, MinWidth)
        SLATE_EVENT(FOnShowKeyClicked, OnClick)
        SLATE_EVENT(FOnShowKeyClicked, OnChanged)
        SLATE_EVENT(FIsShowKeySelected, IsShowKeySelected)
    SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

    FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)override;
    
    virtual bool IsInteractable() const override
    {
        return true;
    }

private:
    mutable FBox2D ClickableBox;
    FVector2D DragStartPosition = FVector2D::ZeroVector;
    
    FShowKey* ShowKey = nullptr;
    TAttribute<float> Height = 20.0f;
    TAttribute<float> MinWidth = 50.0f;
    TAttribute<float> SecondToWidthRatio = 10.0f;
    FOnShowKeyClicked OnClick = nullptr;
    FOnShowKeyClicked OnChanged = nullptr;
    FIsShowKeySelected IsShowKeySelected = nullptr;
};
