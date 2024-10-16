// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "ShowMaker/SShowKeyBox.h"
#include "Widgets/SCompoundWidget.h"

class FShowSequencerEditorHelper;

DECLARE_DELEGATE_OneParam(FOnShowBaseEvent, UShowBase*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowKeyBoxHandler : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SShowKeyBoxHandler) {}
        SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, EditorHelper)
        SLATE_ATTRIBUTE(float, Height)
        SLATE_ATTRIBUTE(float, MinWidth)
        SLATE_EVENT(FOnShowBaseEvent, OnClickedKey)
        SLATE_EVENT(FOnShowBaseEvent, OnChangedKey)
    SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
    void RefreshShowKeyWidgets();

    void OnKeyClicked(UShowBase* ClickedhowBase);
    bool IsShowKeySelected(UShowBase* ShowBase);
    float GetSecondToWidthRatio();

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
    TAttribute<float> Height = 30.0f;
    TAttribute<float> MinWidth = 20.0f;

    FOnShowBaseEvent OnClickedKey = nullptr;
    FOnShowBaseEvent OnChangedKey = nullptr;
    
    TSharedPtr<SVerticalBox> VerticalBox;
    TSharedPtr<IMenu> MenuWindow = nullptr;
    float WidgetWidth = 0.0f;
};
