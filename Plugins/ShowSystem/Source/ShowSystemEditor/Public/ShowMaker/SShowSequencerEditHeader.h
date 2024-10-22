// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/SortedPairArray.h"

class UShowBase;
class FShowSequencerEditorHelper;

DECLARE_DELEGATE_TwoParams(FOnShowBaseEditEvent, TSharedPtr<FShowSequencerEditorHelper>, UShowBase*);
DECLARE_DELEGATE_OneParam(FOnShowSequencerRemoveEvent, TSharedPtr<FShowSequencerEditorHelper>);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIsShowKeySelected, UShowBase*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerEditHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerEditHeader) {}
		SLATE_ATTRIBUTE(float, TitleHeight)
		SLATE_ATTRIBUTE(float, Height)
		SLATE_ATTRIBUTE(float, Width)
		SLATE_EVENT(FOnShowBaseEditEvent, OnShowKeyClicked)
		SLATE_EVENT(FOnShowBaseEditEvent, OnAddShowKeyEvent)
		SLATE_EVENT(FOnShowSequencerRemoveEvent, OnRemoveShowKeyEvent)
		SLATE_EVENT(FIsShowKeySelected, IsShowKeySelected)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void RefreshShowKeyHeaderBoxs(TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr);
	TSharedRef<SWidget> ConstructShowSequencerHeaderWidget(TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper);

	TSharedRef<SWidget> CreateAddKeyMenu();
	TSharedRef<ITableRow> GenerateKeyRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> ShowSequencerMenuBuilder();
	
	void OnAddKeySelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	void OnAddKeySelected(TSharedPtr<FString> SelectedItem);
	void OnAddSequencerKeySelected(TSharedPtr<FString> SelectedItem);
	
	FReply OnRemoveShowKey(TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper, UShowBase* ShowBasePtr);

private:
	FArguments Args;
	TAttribute<float> TitleHeight = 30.0f;
	TAttribute<float> Height = 30.0f;
	TAttribute<float> Width = 100.0f;
	FOnShowBaseEditEvent OnAddShowKeyEvent = nullptr;
	FOnShowSequencerRemoveEvent OnRemoveShowKeyEvent = nullptr;

	TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* ShowSequencerEditorHelperSortMapPtr;
	TArray<TSharedPtr<FString>> KeyOptions;
	TSharedPtr<SVerticalBox> VerticalBox = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> SelectedShowSequencerEditorHelper = nullptr;
};
