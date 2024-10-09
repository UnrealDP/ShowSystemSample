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
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "RunTime/ShowBase.h"
#include "SlateEditorUtils.h"

#define LOCTEXT_NAMESPACE "SShowSequencerScrubPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerScrubPanel::Construct(const SShowSequencerScrubPanel::FArguments& InArgs)
{
	bSliderBeingDragged = false;
	ShowSequencerEditorHelper = InArgs._ShowSequencerEditorHelper;

	ShowViewInputMax = GetSequenceLength();

	FEditorWidgetsModule& EditorWidgetsModule = FModuleManager::Get().LoadModuleChecked<FEditorWidgetsModule>("EditorWidgets");

	FTransportControlArgs TransportControlArgs;
	/*TransportControlArgs.OnForwardPlay.BindSP(this, &YourWidgetClass::HandlePlayButton);
	TransportControlArgs.OnBackwardPlay.BindSP(this, &YourWidgetClass::HandleReverseButton);
	TransportControlArgs.OnStop.BindSP(this, &YourWidgetClass::HandleStopButton);
	TransportControlArgs.OnPause.BindSP(this, &YourWidgetClass::HandlePauseButton);*/

	//TransportControlArgs.OnForwardPlay = InArgs._OnClickedForwardPlay;
	//TransportControlArgs.OnRecord = InArgs._OnClickedRecord;
	//TransportControlArgs.OnBackwardPlay = InArgs._OnClickedBackwardPlay;
	//TransportControlArgs.OnForwardStep = InArgs._OnClickedForwardStep;
	//TransportControlArgs.OnBackwardStep = InArgs._OnClickedBackwardStep;
	//TransportControlArgs.OnForwardEnd = InArgs._OnClickedForwardEnd;
	//TransportControlArgs.OnBackwardEnd = InArgs._OnClickedBackwardEnd;
	//TransportControlArgs.OnToggleLooping = InArgs._OnClickedToggleLoop;
	//TransportControlArgs.OnGetLooping = InArgs._OnGetLooping;
	//TransportControlArgs.OnGetPlaybackMode = InArgs._OnGetPlaybackMode;
	//TransportControlArgs.OnGetRecording = InArgs._OnGetRecording;
	//TransportControlArgs.OnTickPlayback = InArgs._OnTickPlayback;
	//TransportControlArgs.WidgetsToCreate = InArgs._TransportControlWidgetsToCreate.Get(TArray<FTransportControlWidget>());

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
							//EditorWidgetsModule.CreateTransportControl(TransportControlArgs)
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

				//+ SHorizontalBox::Slot()
				//.HAlign(HAlign_Fill)
				//.VAlign(VAlign_Center)
				//.FillWidth(1)
				//.Padding(0.0f)
				//[
				//	SAssignNew(ScrubControlPanel, SScrubControlPanel)
				//		.IsEnabled(true)//this, &SAnimationScrubPanel::DoesSyncViewport)
				//		.Value(this, &SShowSequencerScrubPanel::GetScrubValue)
				//		.NumOfKeys(this, &SShowSequencerScrubPanel::GetNumberOfKeys)
				//		.SequenceLength(this, &SShowSequencerScrubPanel::GetSequenceLength)
				//		.DisplayDrag(this, &SShowSequencerScrubPanel::GetDisplayDrag)
				//		.OnValueChanged(this, &SShowSequencerScrubPanel::OnValueChanged)
				//		.OnBeginSliderMovement(this, &SShowSequencerScrubPanel::OnBeginSliderMovement)
				//		.OnEndSliderMovement(this, &SShowSequencerScrubPanel::OnEndSliderMovement)
				//		.OnClickedForwardPlay(this, &SShowSequencerScrubPanel::OnClick_Forward)
				//		.OnClickedForwardStep(this, &SShowSequencerScrubPanel::OnClick_Forward_Step)
				//		.OnClickedForwardEnd(this, &SShowSequencerScrubPanel::OnClick_Forward_End)
				//		.OnClickedBackwardPlay(this, &SShowSequencerScrubPanel::OnClick_Backward)
				//		.OnClickedBackwardStep(this, &SShowSequencerScrubPanel::OnClick_Backward_Step)
				//		.OnClickedBackwardEnd(this, &SShowSequencerScrubPanel::OnClick_Backward_End)
				//		.OnClickedToggleLoop(this, &SShowSequencerScrubPanel::OnClick_ToggleLoop)
				//		.OnClickedRecord(this, &SShowSequencerScrubPanel::OnClick_Record)
				//		.OnGetLooping(this, &SShowSequencerScrubPanel::IsLoopStatusOn)
				//		.OnGetPlaybackMode(this, &SShowSequencerScrubPanel::GetPlaybackMode)
				//		.OnGetRecording(this, &SShowSequencerScrubPanel::IsRecording)
				//		.ViewInputMin(0)
				//		.ViewInputMax(100)
				//		.bDisplayAnimScrubBarEditing(InArgs._bDisplayAnimScrubBarEditing)
				//		.bAllowZoom(InArgs._bAllowZoom)
				//		.IsRealtimeStreamingMode(this, &SShowSequencerScrubPanel::IsRealtimeStreamingMode)
				//]
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
				return (CurrentPlaybackMode == EPlaybackMode::PlayingForward) ? LOCTEXT("Pause", "Pause") : LOCTEXT("Play", "Play");
					})
				.ContentPadding(0.0f)
				.IsFocusable(true)
				[
					SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseSubduedForeground())
						.Image_Lambda([this]() -> const FSlateBrush* {
						return (CurrentPlaybackMode == EPlaybackMode::PlayingForward)
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
	if (CurrentPlaybackMode == EPlaybackMode::PlayingForward)
	{
		CurrentPlaybackMode = EPlaybackMode::Stopped;
		UE_LOG(LogTemp, Log, TEXT("Paused"));
		// Pause 로직 추가
	}
	else
	{
		CurrentPlaybackMode = EPlaybackMode::PlayingForward;
		UE_LOG(LogTemp, Log, TEXT("Playing"));
		// Play 로직 추가
	}

	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::HandleReverseButton()
{
	CurrentPlaybackMode = EPlaybackMode::PlayingReverse;
	UE_LOG(LogTemp, Log, TEXT("Reversing"));
	// Reverse 로직 추가
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::HandleStopButton()
{
	CurrentPlaybackMode = EPlaybackMode::Stopped;
	UE_LOG(LogTemp, Log, TEXT("Stopped"));
	// Stop 로직 추가
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

FReply SShowSequencerScrubPanel::OnClick_Forward_Step()
{
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Forward_End()
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if (PreviewInstance)
	{
		PreviewInstance->SetPlaying(false);
		PreviewInstance->SetPosition(PreviewInstance->GetLength(), false);
	}

	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Backward_Step()
{
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Backward_End()
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if (PreviewInstance)
	{
		PreviewInstance->SetPlaying(false);
		PreviewInstance->SetPosition(0.f, false);
	}
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Forward()
{
	if (ShowSequencerEditorHelper)
	{
		ShowSequencerEditorHelper->Play();
	}

	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Backward()
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if (PreviewInstance)
	{
		bool bIsReverse = PreviewInstance->IsReverse();
		bool bIsPlaying = PreviewInstance->IsPlaying();
		// if currently playing forward, just simply turn on reverse
		if (!bIsReverse && bIsPlaying)
		{
			PreviewInstance->SetReverse(true);
		}
		else if (bIsPlaying)
		{
			PreviewInstance->SetPlaying(false);
		}
		else
		{
			//if we're at the beginning of the animation, jump back to the end before playing
			if (GetScrubValue() <= 0.0f)
			{
				PreviewInstance->SetPosition(GetSequenceLength(), false);
			}

			PreviewInstance->SetPlaying(true);
			PreviewInstance->SetReverse(true);
		}
	}
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_ToggleLoop()
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if (PreviewInstance)
	{
		bool bIsLooping = PreviewInstance->IsLooping();
		PreviewInstance->SetLooping(!bIsLooping);
	}
	return FReply::Handled();
}

FReply SShowSequencerScrubPanel::OnClick_Record()
{
	//StaticCastSharedRef<FAnimationEditorPreviewScene>(GetPreviewScene())->RecordAnimation();

	return FReply::Handled();
}

bool SShowSequencerScrubPanel::IsLoopStatusOn() const
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	return (PreviewInstance && PreviewInstance->IsLooping());
}

EPlaybackMode::Type SShowSequencerScrubPanel::GetPlaybackMode() const
{
	return EPlaybackMode::Stopped;
}

bool SShowSequencerScrubPanel::IsRecording() const
{
	return false;
	//return StaticCastSharedRef<FAnimationEditorPreviewScene>(GetPreviewScene())->IsRecording();
}

bool SShowSequencerScrubPanel::IsRealtimeStreamingMode() const
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	return (!(PreviewInstance && PreviewInstance->GetCurrentAsset()));
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

	if (UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance())
	{
		PreviewInstance->SetPlaying(false);
	}
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

bool SShowSequencerScrubPanel::DoesSyncViewport() const
{
	/*UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();

	return ((ShowSequncer == nullptr && PreviewInstance) || (ShowSequncer && PreviewInstance && PreviewInstance->GetCurrentAsset() == ShowSequncer));*/
	return false;
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

class UAnimSingleNodeInstance* SShowSequencerScrubPanel::GetPreviewInstance() const
{
	return nullptr;
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
