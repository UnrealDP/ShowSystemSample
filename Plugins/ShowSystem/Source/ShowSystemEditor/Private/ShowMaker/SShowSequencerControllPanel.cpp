// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerControllPanel.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "SShowSequencerControllPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerControllPanel::Construct(const FArguments& InArgs)
{
	ShowSequencerState = InArgs._ShowSequencerState;
	OnPlay = InArgs._OnPlay;

	ChildSlot
	[
		SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				// Play/Pause 버튼
				SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "Animation.PlayControlsButton")
					.OnClicked(this, &SShowSequencerControllPanel::HandlePlayPauseButton)
					.Visibility(EVisibility::Visible)
					.ToolTipText_Lambda([this]() -> FText {
					return (ShowSequencerState.Get() == EShowSequencerState::ShowSequencer_Playing)
						? LOCTEXT("Pause", "Pause")
						: LOCTEXT("Play", "Play");
						})
					.ContentPadding(0.0f)
					.IsFocusable(true)
					[
						SNew(SImage)
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Image_Lambda([this]() -> const FSlateBrush* {
							return (ShowSequencerState.Get() == EShowSequencerState::ShowSequencer_Playing)
								? FAppStyle::Get().GetBrush("Animation.Pause") // 상태에 따른 아이콘 전환
								: FAppStyle::Get().GetBrush("Animation.Forward");
								})
					]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				// Reverse 버튼
				SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "Animation.PlayControlsButton")
					.OnClicked(this, &SShowSequencerControllPanel::HandleReverseButton)
					.Visibility(EVisibility::Visible)
					.ToolTipText(LOCTEXT("Reverse", "Reverse"))
					.ContentPadding(0.0f)
					.IsFocusable(true)
					[
						SNew(SImage)
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Image(FAppStyle::Get().GetBrush("Animation.Backward"))
					]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				// Stop 버튼
				SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "Animation.PlayControlsButton")
					.OnClicked(this, &SShowSequencerControllPanel::HandleStopButton)
					.Visibility(EVisibility::Visible)
					.ToolTipText(LOCTEXT("Stop", "Stop"))
					.ContentPadding(0.0f)
					.IsFocusable(true)
					[
						SNew(SImage)
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Image(FAppStyle::Get().GetBrush("Animation.Stop"))
					]
			]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


FReply SShowSequencerControllPanel::HandlePlayPauseButton()
{
	if (OnPlay.IsBound())
	{
		OnPlay.Execute();
	}
	return FReply::Handled();
}

FReply SShowSequencerControllPanel::HandleReverseButton()
{
	return FReply::Handled();
}

FReply SShowSequencerControllPanel::HandleStopButton()
{
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE