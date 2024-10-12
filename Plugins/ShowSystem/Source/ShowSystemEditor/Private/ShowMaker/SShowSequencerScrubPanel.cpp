// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerScrubPanel.h"
#include "EditorWidgetsModule.h"
#include "SlateOptMacros.h"
#include "Widgets/SBoxPanel.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Animation/AnimBlueprint.h"
#include "AnimPreviewInstance.h"
#include "SScrubControlPanel.h"
#include "ScopedTransaction.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimSequenceHelpers.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "RunTime/ShowBase.h"
#include "SlateEditorUtils.h"

#define LOCTEXT_NAMESPACE "SShowSequencerScrubPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerScrubPanel::Construct(const SShowSequencerScrubPanel::FArguments& InArgs)
{
	bSliderBeingDragged = false;
	ShowSequencerEditorHelper = InArgs._ShowSequencerEditorHelper;

	ShowSequencerState = TAttribute<EShowSequencerState>::Create(TAttribute<EShowSequencerState>::FGetter::CreateLambda([this]()
		{
			return ShowSequencerEditorHelper->EditShowSequencer->GetShowSequencerState();
		}));


	ShowViewInputMax = GetSequenceLength();

	ChildSlot
		[
			SNew(SHorizontalBox)
				.AddMetaData<FTagMetaData>(TEXT("AnimScrub.Scrub"))

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
						.Padding(0.0f)
						.BorderImage(FAppStyle::GetBrush("NoBorder"))
						[
							CreateCustomTransportControl()
						]
				]

				SLATE_HORIZONTAL_SLOT(16.0f, 16.0f)

				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.FillWidth(1)
				.Padding(FMargin(0.0f, 0.0f))
				[
					SNew(SBorder)
						[
							SAssignNew(ShowScrubWidget, SScrubWidget)
								.Clipping(EWidgetClipping::ClipToBounds)
								.Value(this, &SShowSequencerScrubPanel::GetScrubValue)
								.NumOfKeys(this, &SShowSequencerScrubPanel::GetNumberOfKeys)
								.SequenceLength(this, &SShowSequencerScrubPanel::GetSequenceLength)
								.DisplayDrag(&SShowSequencerScrubPanel::GetDisplayDrag)
								.bDisplayAnimScrubBarEditing(false)
								.OnValueChanged(this, &SShowSequencerScrubPanel::OnValueChanged)
								.OnBeginSliderMovement(this, &SShowSequencerScrubPanel::OnBeginSliderMovement)
								.OnEndSliderMovement(this, &SShowSequencerScrubPanel::OnEndSliderMovement)
								.ViewInputMin(this, &SShowSequencerScrubPanel::GetViewInputMin)
								.ViewInputMax(this, &SShowSequencerScrubPanel::GetViewInputMax)
								.bAllowZoom(InArgs._bAllowZoom)
								.OnSetInputViewRange(this, &SShowSequencerScrubPanel::OnSetInputViewRange)
								.DraggableBars(DraggableBars)
								.OnBarDrag(this, &SShowSequencerScrubPanel::OnScrubBarDrag)
								.OnBarCommit(this, &SShowSequencerScrubPanel::OnScrubBarCommit)
						]
				]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SShowSequencerScrubPanel::CreateCustomTransportControl()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			// Play/Pause 버튼
			SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "Animation.PlayControlsButton")
				.OnClicked(this, &SShowSequencerScrubPanel::HandlePlayPauseButton)
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
				.OnClicked(this, &SShowSequencerScrubPanel::HandleReverseButton)
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
				.OnClicked(this, &SShowSequencerScrubPanel::HandleStopButton)
				.Visibility(EVisibility::Visible)
				.ToolTipText(LOCTEXT("Stop", "Stop"))
				.ContentPadding(0.0f)
				.IsFocusable(true)
				[
					SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Image(FAppStyle::Get().GetBrush("Animation.Stop"))
				]
		];
}

FReply SShowSequencerScrubPanel::HandlePlayPauseButton()
{
	ShowSequencerEditorHelper->Play();
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::HandleReverseButton()
{
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::HandleStopButton()
{
	return FReply::Handled();
}

float SShowSequencerScrubPanel::GetViewInputMin() const 
{ 
	return ShowViewInputMin; 
}

float SShowSequencerScrubPanel::GetViewInputMax() const 
{ 
	return ShowViewInputMax; 
}

void SShowSequencerScrubPanel::OnValueChanged(float NewValue)
{
	if (!ShowSequencerEditorHelper->EditShowSequencer)
	{
		return;
	}

	//ShowSequencerEditorHelper->EditShowSequencer->SetPassedTime(NewValue);
}

// make sure viewport is freshes
void SShowSequencerScrubPanel::OnBeginSliderMovement()
{
	bSliderBeingDragged = true;
}

void SShowSequencerScrubPanel::OnEndSliderMovement(float NewValue)
{
	bSliderBeingDragged = false;
}

float SShowSequencerScrubPanel::GetScrubValue() const
{
	if (!ShowSequencerEditorHelper->EditShowSequencer)
	{
		return 0.0f;
	}

	return ShowSequencerEditorHelper->EditShowSequencer->GetPassedTime();
}

uint32 SShowSequencerScrubPanel::GetNumberOfKeys() const
{
	float SequenceLength = GetSequenceLength();
	int32 NumKeys = (int32)(SequenceLength / 0.0333f);
	return NumKeys;
}

float SShowSequencerScrubPanel::GetSequenceLength() const
{
	return CrrShowSequenceLength;
}

void SShowSequencerScrubPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (!ShowSequencerEditorHelper->EditShowSequencer)
	{
		return;
	}

	float SequenceLength = 0.0f;
	TArray<TObjectPtr<UShowBase>>* Shows = ShowSequencerEditorHelper->EditShowSequencer->EditorGetShowKeys();
	if (Shows)
	{
		for (TObjectPtr<UShowBase>& ShowBase : *Shows)
		{
			if (!ShowBase)
			{
				continue;
			}

			float StartTime = ShowBase->GetStartTime();
			float Length = ShowBase->GetLength();

			SequenceLength = FMath::Max(SequenceLength, StartTime + Length);
		}
	}

	if (CrrShowSequenceLength != SequenceLength || ShowViewInputMax == FLT_MAX)
	{
		float ZoomRate = 1.0f;
		if (ShowViewInputMax != FLT_MAX)
		{
			ZoomRate = ShowViewInputMax / CrrShowSequenceLength;
		}
		
		CrrShowSequenceLength = SequenceLength;
		ShowViewInputMax = CrrShowSequenceLength * ZoomRate;
	}
}

bool SShowSequencerScrubPanel::GetDisplayDrag() const
{
	if (!ShowSequencerEditorHelper->EditShowSequencer)
	{
		return false;
	}

	return true;
}

void SShowSequencerScrubPanel::OnSetInputViewRange(float NewViewMinInput, float NewViewMaxInput)
{
	ShowViewInputMax = NewViewMaxInput - NewViewMinInput;
}

void SShowSequencerScrubPanel::OnScrubBarDrag(int32 BarIndex, float NewPosition)
{
}

void SShowSequencerScrubPanel::OnScrubBarCommit(int32 BarIndex, float FinalPosition)
{
}
