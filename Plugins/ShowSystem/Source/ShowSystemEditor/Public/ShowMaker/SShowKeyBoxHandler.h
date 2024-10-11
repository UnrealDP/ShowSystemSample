// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "ShowMaker/SShowKeyBox.h"
#include "Widgets/SCompoundWidget.h"

class FShowSequencerEditorHelper;

DECLARE_DELEGATE_OneParam(FOnShowKeyEvent, FShowKey*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowKeyBoxHandler : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SShowKeyBoxHandler) {}
        SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, ShowSequencerEditorHelper)
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
    void RefreshShowKeyWidgets();

    TSharedRef<SWidget> CreateAddKeyMenu();
    TSharedRef<ITableRow> GenerateKeyRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
    void OnAddKeySelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
    void OnKeyClicked(FShowKey* ClickedKey);
    bool IsShowKeySelected(FShowKey* ShowKey);

private:
    TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper = nullptr;
    TAttribute<float> Height = 30.0f;
    TAttribute<float> MinWidth = 50.0f;
    TAttribute<float> SecondToWidthRatio = 10.0f;

    FOnShowKeyEvent OnAddKey = nullptr;
    FOnShowKeyEvent OnRemoveKey = nullptr;
    FOnShowKeyEvent OnClickedKey = nullptr;
    FOnShowKeyEvent OnChangedKey = nullptr;
    
    TSharedPtr<SVerticalBox> VerticalBox;
    TArray<TSharedPtr<SShowKeyBox>> ShowKeyBoxes;
    TSharedPtr<IMenu> MenuWindow = nullptr;

    TArray<TSharedPtr<FString>> KeyOptions;
};
