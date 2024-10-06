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
        SLATE_EVENT(FOnShowKeyEvent, OnClickedKey)
        SLATE_EVENT(FOnShowKeyEvent, OnChangedKey)
    SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

    TSharedRef<SWidget> CreateMenuBar();
    void GenerateMenu(FMenuBuilder& MenuBuilder);

    void OnKeyClicked(FShowKey* ClickedKey);
    void OnKeyTypeSelected(FString SelectedKeyType);

private:
    TArray<FShowKey*> ShowKeys;
    TAttribute<float> Height = 30.0f;
    TAttribute<float> MinWidth = 50.0f;
    TAttribute<float> SecondToWidthRatio = 10.0f;

    FOnShowKeyEvent OnAddKey;
    FOnShowKeyEvent OnRemoveKey;
    FOnShowKeyEvent OnClickedKey;
    FOnShowKeyEvent OnChangedKey;
    
    TArray<TSharedPtr<SShowKeyBox>> ShowKeyBoxes;
    TSharedPtr<IMenu> MenuWindow = nullptr;
};
