// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "RunTime/ShowSequencer.h"
#include "Data/ActionBaseData.h"

struct FSkillData;
struct FSkillShowData;
class FShowSequencerEditorHelper;
class UActionBase;
class SShowSequencerEditHeader;
class SShowKeyBoxHandler;

DECLARE_DELEGATE_TwoParams(FOnShowBaseEditEvent, TSharedPtr<FShowSequencerEditorHelper>, UShowBase*);
DECLARE_DELEGATE_OneParam(FOnShowSequencerRemoveEvent, TSharedPtr<FShowSequencerEditorHelper>);
DECLARE_DELEGATE(FOnPlay);

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SShowActionControllPanels : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowActionControllPanels) {}
		SLATE_EVENT(FOnShowBaseEditEvent, OnAddKey)
		SLATE_EVENT(FOnShowBaseEditEvent, OnSelectedKey)
		SLATE_EVENT(FOnShowSequencerRemoveEvent, OnRemoveKey)
		SLATE_EVENT(FOnPlay, OnPlay)
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
	void ChangeShow(EActionState ActionState, TObjectPtr<UShowSequencer> NewShow);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FArguments Args;

	TSharedPtr<SShowKeyBoxHandler> ShowKeyBoxHandler = nullptr;
	TSharedPtr<SHorizontalBox> HorizontalBox = nullptr;
	TSharedPtr<SShowSequencerEditHeader> ShowSequencerEditHeader = nullptr;
	TAttribute<EShowSequencerState> ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;

	UActionBase* CrrAction = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> CastEditorHelper = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> ExecEditorHelper = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> FinishEditorHelper = nullptr;
	TMap<FString, TSharedPtr<FShowSequencerEditorHelper>> ShowSequencerEditorHelperMap;

	bool IsUpdateKey = false;
	FOnShowBaseEditEvent OnAddKey = nullptr;
	FOnShowBaseEditEvent OnSelectedKey = nullptr;
	FOnShowSequencerRemoveEvent OnRemoveKey = nullptr;
};
