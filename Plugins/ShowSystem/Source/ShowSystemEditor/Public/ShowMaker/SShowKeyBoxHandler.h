// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "ShowMaker/SShowKeyBox.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnShowKeyEvent, FShowKey*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowKeyBoxHandler : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SShowKeyBoxHandler) {}
        SLATE_ARGUMENT(TArray<FShowKey*>, ShowKeys)
        SLATE_ATTRIBUTE(float, Height)
        SLATE_ATTRIBUTE(float, MinWidth)
        SLATE_ATTRIBUTE(float, SecondToWidthRatio)
        SLATE_EVENT(FOnShowKeyEvent, OnAddKey)
        SLATE_EVENT(FOnShowKeyEvent, OnRemoveKey)
    SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

    /*virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
        FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;*/

    TSharedRef<SWidget> CreateMenuBar();
    void GenerateMenu(FMenuBuilder& MenuBuilder);

    /*FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    FReply OnAddKeyClicked();*/
    void OnKeyClicked(FShowKey* ClickedKey);
    void OnKeyTypeSelected(FString SelectedKeyType);
    FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
    TArray<FShowKey*> ShowKeys;
    TAttribute<float> Height;
    TAttribute<float> MinWidth;
    TAttribute<float> SecondToWidthRatio;
    FOnShowKeyEvent OnAddKey;
    FOnShowKeyEvent OnRemoveKey;
    TArray<TSharedPtr<SShowKeyBox>> ShowKeyBoxes;
    FShowKey* SelectedKey;
    TSharedPtr<IMenu> MenuWindow;
};
