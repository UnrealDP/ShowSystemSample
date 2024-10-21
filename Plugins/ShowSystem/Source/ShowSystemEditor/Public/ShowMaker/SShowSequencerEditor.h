// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/SortedPairArray.h"

class SShowKeyBoxHandler;
class FShowSequencerEditorHelper;
class SShowSequencerEditHeader;

DECLARE_DELEGATE_OneParam(FOnShowBaseEvent, UShowBase*);
DECLARE_DELEGATE(FOnShowRemoveEvent);
DECLARE_DELEGATE(FOnKeyDownSpace);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerEditor) {}
		SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, EditorHelper)
		SLATE_ATTRIBUTE(float, Height)
		SLATE_ATTRIBUTE(float, MinWidth)
		SLATE_ARGUMENT(TAttribute<EShowSequencerState>, ShowSequencerState)
		SLATE_EVENT(FOnShowBaseEvent, OnAddKey)
		SLATE_EVENT(FOnShowRemoveEvent, OnRemoveKey)
		SLATE_EVENT(FOnShowBaseEvent, OnClickedKey)
		SLATE_EVENT(FOnShowBaseEvent, OnChangedKey)
		SLATE_EVENT(FOnKeyDownSpace, OnKeyDownSpace)		
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> ConstructLeftWidget(const FArguments& InArgs);
	TSharedRef<SWidget> ConstructRightWidget(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	FOnShowBaseEvent OnAddKey;
	FOnShowRemoveEvent OnRemoveKey;
	
	TSharedPtr<SShowKeyBoxHandler> ShowKeyBoxHandler = nullptr;
	TSharedPtr<SShowSequencerEditHeader> ShowSequencerEditHeader = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
	FOnKeyDownSpace OnKeyDownSpace = nullptr;
	TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>> ShowSequencerEditorHelperSortMap;
	bool IsUpdateKey = false;
};
