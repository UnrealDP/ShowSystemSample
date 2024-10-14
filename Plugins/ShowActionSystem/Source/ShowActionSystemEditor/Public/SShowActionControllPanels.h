// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RunTime/ShowSequencer.h"

struct FSkillData;
struct FSkillShowData;
class FShowSequencerEditorHelper;
class UActionBase;
class SShowSequencerEditHeader;

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SShowActionControllPanels : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowActionControllPanels)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> ConstructLeftWidget(const FArguments& InArgs);
	void ConstructRightWidget(const FArguments& InArgs);
	TSharedRef<SWidget> ConstructShowSequencerWidget(const FArguments& InArgs, TSharedPtr<FShowSequencerEditorHelper> EditorHelper);

    void SelectAction(
		UActionBase* InAction, 
		TObjectPtr<UShowSequencer> InCastShow,
		TObjectPtr<UShowSequencer> InExecShow,
		TObjectPtr<UShowSequencer> InFinishShow);

	FArguments Args;

	TSharedPtr<SHorizontalBox> HorizontalBox = nullptr;
	TSharedPtr<SShowSequencerEditHeader> ShowSequencerEditHeader = nullptr;
	TAttribute<EShowSequencerState> ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;

	UActionBase* CrrAction = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> CastEditorHelper = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> ExecEditorHelper = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> FinishEditorHelper = nullptr;
	TMap<FString, TSharedPtr<FShowSequencerEditorHelper>> InShowSequencerEditorHelperMap;
};
