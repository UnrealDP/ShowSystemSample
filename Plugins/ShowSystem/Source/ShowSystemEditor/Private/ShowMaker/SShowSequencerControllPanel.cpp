// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerControllPanel.h"
#include "SlateOptMacros.h"
#include "SSliderWithEditBox.h"

#define LOCTEXT_NAMESPACE "SShowSequencerControllPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerControllPanel::Construct(const FArguments& InArgs)
{
	bIsPlaying = InArgs._bIsPlaying;
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
					return (bIsPlaying.Get() == true)
						? LOCTEXT("Pause", "Pause")
						: LOCTEXT("Play", "Play");
						})
					.ContentPadding(0.0f)
					.IsFocusable(true)
					[
						SNew(SImage)
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.Image_Lambda([this]() -> const FSlateBrush* {

							return (bIsPlaying.Get() == true)
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

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				// Time Scale Slider
				SNew(SSliderWithEditBox)
					.MinValue(0.0f)
					.MaxValue(3.0f)
					.InitialValue(1.0f)
					.EditBoxPosition(SSliderWithEditBox::EEditBoxPosition::Right)
					.OnSliderValueChanged(InArgs._OnTimeScaleValueChanged)
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