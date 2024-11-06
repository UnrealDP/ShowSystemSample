// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE(FOnPlay);
DECLARE_DELEGATE_OneParam(FOnTimeScaleValueChanged, float);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerControllPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerControllPanel) {}
		SLATE_ATTRIBUTE(bool, bIsPlaying)
		SLATE_EVENT(FOnPlay, OnPlay)
		SLATE_EVENT(FOnTimeScaleValueChanged, OnTimeScaleValueChanged)		
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	FReply HandlePlayPauseButton();
	FReply HandleReverseButton();
	FReply HandleStopButton();

	TAttribute<bool> bIsPlaying = false;
	FOnPlay OnPlay;
};
