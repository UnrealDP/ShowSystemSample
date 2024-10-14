// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"

DECLARE_DELEGATE(FOnPlay);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerControllPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerControllPanel) {}
		SLATE_ARGUMENT(TAttribute<EShowSequencerState>, ShowSequencerState)
		SLATE_EVENT(FOnPlay, OnPlay)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FReply HandlePlayPauseButton();
	FReply HandleReverseButton();
	FReply HandleStopButton();

	TAttribute<EShowSequencerState> ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;	
	FOnPlay OnPlay;
};
