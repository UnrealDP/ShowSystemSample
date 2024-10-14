// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FShowKey;
class FShowSequencerEditorHelper;

DECLARE_DELEGATE_TwoParams(FOnShowKeyEditEvent, TSharedPtr<FShowSequencerEditorHelper>, FShowKey*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerEditHeader : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerEditHeader) {}
		SLATE_ATTRIBUTE(float, Height)
		SLATE_ATTRIBUTE(float, Width)
		SLATE_EVENT(FOnShowKeyEditEvent, OnAddShowKeyEvent)
		SLATE_EVENT(FOnShowKeyEditEvent, OnRemoveShowKeyEvent)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void RefreshShowKeyHeaderBoxs(TMap<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperMapPtr);

	TSharedRef<SWidget> CreateAddKeyMenu();
	TSharedRef<ITableRow> GenerateKeyRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<SWidget> ShowSequencerMenuBuilder();
	
	void OnAddKeySelected(TSharedPtr<FString> SelectedItem, ESelectInfo::Type SelectInfo);
	void OnAddKeySelected(TSharedPtr<FString> SelectedItem);
	void OnAddSequencerKeySelected(TSharedPtr<FString> SelectedItem);

	TAttribute<float> Height = 30.0f;
	TAttribute<float> Width = 100.0f;
	FOnShowKeyEditEvent OnAddShowKeyEvent = nullptr;
	FOnShowKeyEditEvent OnRemoveShowKeyEvent = nullptr;

	TMap<FString, TSharedPtr<FShowSequencerEditorHelper>>* ShowSequencerEditorHelperMapPtr = nullptr;
	TArray<TSharedPtr<FString>> KeyOptions;
	TSharedPtr<SVerticalBox> VerticalBox = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> SelectedShowSequencerEditorHelper = nullptr;
};
