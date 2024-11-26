// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Misc/SortedPairArray.h"
#include "SShowActionControllPanels.h"

class FShowSequencerEditorHelper;
class UActionBase;

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SActionShowKeyPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SActionShowKeyPanel) {}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void SetShowSequencerEditorHelperSortMap(
		const SShowActionControllPanels::FArguments& InArgs,
		UActionBase* InCrrActionPtr, 
		TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* InShowSequencerEditorHelperSortMapPtr);

private:
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);

	void ConstructShowSequencerWidget(
		const SShowActionControllPanels::FArguments& InArgs,
		TSharedPtr<FShowSequencerEditorHelper>& EditorHelper,
		TAttribute<float> ExHeight);

private:
	UActionBase* CrrActionPtr = nullptr;
	TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>>* ShowSequencerEditorHelperSortMapPtr;

	TSharedPtr<SVerticalBox> VerticalBox = nullptr;
	float ScrubBoardTotalValue = 0.0f;
	float WidgetWidth = 0.0f;
};
