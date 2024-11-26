// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RunTime/ShowSequencer.h"
#include "Data/ActionBaseData.h"
#include "Misc/SortedPairArray.h"

struct FSkillData;
struct FSkillShowData;
class FShowSequencerEditorHelper;
class UActionBase;
class SShowSequencerEditHeader;
class SShowKeyBoxHandler;
class SActionShowKeyPanel;

DECLARE_DELEGATE_TwoParams(FOnShowBaseEditEvent, TSharedPtr<FShowSequencerEditorHelper>, UShowBase*);
DECLARE_DELEGATE_OneParam(FOnShowSequencerRemoveEvent, TSharedPtr<FShowSequencerEditorHelper>);
DECLARE_DELEGATE(FOnPlay);
DECLARE_DELEGATE_RetVal_OneParam(bool, FIsShowKeySelected, UShowBase*);
DECLARE_DELEGATE_OneParam(FOnTimeScaleValueChanged, float);

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SShowActionControllPanels : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowActionControllPanels) {}
		SLATE_ATTRIBUTE(float, Height)
		SLATE_ATTRIBUTE(bool, bIsPlaying)
		SLATE_EVENT(FOnShowBaseEditEvent, OnAddKey)
		SLATE_EVENT(FOnShowBaseEditEvent, OnSelectedKey)
		SLATE_EVENT(FOnShowSequencerRemoveEvent, OnRemoveKey)
		SLATE_EVENT(FOnPlay, OnPlay)
		SLATE_EVENT(FIsShowKeySelected, IsShowKeySelected)
		SLATE_EVENT(FOnTimeScaleValueChanged, OnTimeScaleValueChanged)		
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> ConstructLeftWidget(const FArguments& InArgs);
	void ConstructRightWidget(UActionBase* InCrrActionPtr, const FArguments& InArgs);
	TSharedRef<SWidget> ConstructShowSequencerWidget(
		const FArguments& InArgs, 
		TSharedPtr<FShowSequencerEditorHelper>& EditorHelper, 
		float ExHeight);

	void RefreshShowActionControllPanels(UActionBase* InCrrActionPtr, TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	FArguments Args;

	UActionBase* CrrActionPtr;
	TSharedPtr<SHorizontalBox> HorizontalBox = nullptr;
	TSharedPtr<SShowSequencerEditHeader> ShowSequencerEditHeader = nullptr;
	TSharedPtr<SActionShowKeyPanel> ActionShowKeyPanel = nullptr;

	TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* ShowSequencerEditorHelperSortMapPtr;

	TAttribute<float> Height = 20.0f;
	bool IsUpdateKey = false;
	FOnShowBaseEditEvent OnAddKey = nullptr;
	FOnShowBaseEditEvent OnSelectedKey = nullptr;
	FOnShowSequencerRemoveEvent OnRemoveKey = nullptr;
};
