// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "IPersonaPreviewScene.h"
#include "Input/Reply.h"
#include "Widgets/SCompoundWidget.h"
#include "SScrubWidget.h"
#include "ITransportControl.h"

class FShowSequencerEditorHelper;
class UShowSequencer;
class SScrubControlPanel;

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerScrubPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerScrubPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, ShowSequencerEditorHelper)
		SLATE_ARGUMENT(bool, bDisplayAnimScrubBarEditing)
		SLATE_ARGUMENT(bool, bAllowZoom)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	TSharedRef<SWidget> CreateCustomTransportControl();
	FReply HandlePlayPauseButton();
	FReply HandleReverseButton();
	FReply HandleStopButton();

	// SWidget interface
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	// End of SWidget interface

protected:
	float GetViewInputMin() const;
	float GetViewInputMax() const;
	virtual void OnValueChanged(float NewValue);

	void OnBeginSliderMovement();
	void OnEndSliderMovement(float NewValue);
	void OnSetInputViewRange(float NewViewMinInput, float NewViewMaxInput);
	void OnScrubBarDrag(int32 BarIndex, float NewPosition);
	void OnScrubBarCommit(int32 BarIndex, float FinalPosition);

	float GetScrubValue() const;

	uint32 GetNumberOfKeys() const;
	float GetSequenceLength() const;

	bool GetDisplayDrag() const;	

	TSharedPtr<SScrubWidget > ShowScrubWidget = nullptr;
	bool bSliderBeingDragged = false;
	TSharedPtr<SScrubControlPanel> ScrubControlPanel = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> ShowSequencerEditorHelper = nullptr;
	TArray<float> DraggableBars;
	float ShowViewInputMin = 0.0f;
	float ShowViewInputMax = FLT_MAX;
	float CrrShowSequenceLength = 0.0f;
	EPlaybackMode::Type CurrentPlaybackMode = EPlaybackMode::Stopped;
};
