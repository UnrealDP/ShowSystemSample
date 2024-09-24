// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/SShowSequencerScrubPanel.h"
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
#include "RunTime/ShowSequencer.h"
#include "Animation/DebugSkelMeshComponent.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowSequencerScrubPanel::Construct(const SShowSequencerScrubPanel::FArguments& InArgs)
{
	bSliderBeingDragged = false;
	EditShowSequencer = InArgs._EditShowSequencer;
	OnSetInputViewRange = InArgs._OnSetInputViewRange;

	this->ChildSlot
		[
			SNew(SHorizontalBox)
				.AddMetaData<FTagMetaData>(TEXT("AnimScrub.Scrub"))
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				.FillWidth(1)
				.Padding(0.0f)
				[
					SAssignNew(ScrubControlPanel, SScrubControlPanel)
						.IsEnabled(true)//this, &SAnimationScrubPanel::DoesSyncViewport)
						.Value(this, &SShowSequencerScrubPanel::GetScrubValue)
						.NumOfKeys(this, &SShowSequencerScrubPanel::GetNumberOfKeys)
						.SequenceLength(this, &SShowSequencerScrubPanel::GetSequenceLength)
						.DisplayDrag(this, &SShowSequencerScrubPanel::GetDisplayDrag)
						.OnValueChanged(this, &SShowSequencerScrubPanel::OnValueChanged)
						.OnBeginSliderMovement(this, &SShowSequencerScrubPanel::OnBeginSliderMovement)
						.OnEndSliderMovement(this, &SShowSequencerScrubPanel::OnEndSliderMovement)
						.OnClickedForwardPlay(this, &SShowSequencerScrubPanel::OnClick_Forward)
						.OnClickedForwardStep(this, &SShowSequencerScrubPanel::OnClick_Forward_Step)
						.OnClickedForwardEnd(this, &SShowSequencerScrubPanel::OnClick_Forward_End)
						.OnClickedBackwardPlay(this, &SShowSequencerScrubPanel::OnClick_Backward)
						.OnClickedBackwardStep(this, &SShowSequencerScrubPanel::OnClick_Backward_Step)
						.OnClickedBackwardEnd(this, &SShowSequencerScrubPanel::OnClick_Backward_End)
						.OnClickedToggleLoop(this, &SShowSequencerScrubPanel::OnClick_ToggleLoop)
						.OnClickedRecord(this, &SShowSequencerScrubPanel::OnClick_Record)
						.OnGetLooping(this, &SShowSequencerScrubPanel::IsLoopStatusOn)
						.OnGetPlaybackMode(this, &SShowSequencerScrubPanel::GetPlaybackMode)
						.OnGetRecording(this, &SShowSequencerScrubPanel::IsRecording)
						.ViewInputMin(InArgs._ViewInputMin)
						.ViewInputMax(InArgs._ViewInputMax)
						.bDisplayAnimScrubBarEditing(InArgs._bDisplayAnimScrubBarEditing)
						.OnSetInputViewRange(InArgs._OnSetInputViewRange)
						.OnCropAnimSequence(this, &SShowSequencerScrubPanel::OnCropAnimSequence)
						.OnAddAnimSequence(this, &SShowSequencerScrubPanel::OnInsertAnimSequence)
						.OnAppendAnimSequence(this, &SShowSequencerScrubPanel::OnAppendAnimSequence)
						.OnReZeroAnimSequence(this, &SShowSequencerScrubPanel::OnReZeroAnimSequence)
						.bAllowZoom(InArgs._bAllowZoom)
						.IsRealtimeStreamingMode(this, &SShowSequencerScrubPanel::IsRealtimeStreamingMode)
				]
		];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION


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
	if (EditShowSequencer)
	{
		EShowSequencerState showSequencerState  = EditShowSequencer->GetShowSequencerState();
		switch (showSequencerState)
		{
		case EShowSequencerState::ShowSequencer_Wait:
			EditShowSequencer->Play();
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			EditShowSequencer->UnPause();
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencer->Play();
			break;
		default:
			break;
		}
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
	if (UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance())
	{
		PreviewInstance->SetPosition(NewValue);
	}
	else
	{
		UAnimInstance* Instance;
		FAnimBlueprintDebugData* DebugData;
		if (GetAnimBlueprintDebugData(/*out*/ Instance, /*out*/ DebugData))
		{
			DebugData->SetSnapshotIndexByTime(Instance, NewValue);
		}
	}
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

uint32 SShowSequencerScrubPanel::GetNumberOfKeys() const
{
	//if (DoesSyncViewport())
	//{
	//	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//	float Length = PreviewInstance->GetLength();
	//	// if anim sequence, use correct num frames
	//	int32 NumKeys = (int32)(Length / 0.0333f);
	//	if (PreviewInstance->GetCurrentAsset())
	//	{
	//		if (PreviewInstance->GetCurrentAsset()->IsA(UAnimSequenceBase::StaticClass()))
	//		{
	//			NumKeys = CastChecked<UAnimSequenceBase>(PreviewInstance->GetCurrentAsset())->GetNumberOfSampledKeys();
	//		}
	//		else if (PreviewInstance->GetCurrentAsset()->IsA(UBlendSpace::StaticClass()))
	//		{
	//			// Blendspaces dont display frame notches, so just return 0 here
	//			NumKeys = 0;
	//		}
	//	}
	//	return NumKeys;
	//}
	//else if (LockedSequence)
	//{
	//	return LockedSequence->GetNumberOfSampledKeys();
	//}
	//else
	//{
	//	UAnimInstance* Instance;
	//	FAnimBlueprintDebugData* DebugData;
	//	if (GetAnimBlueprintDebugData(/*out*/ Instance, /*out*/ DebugData))
	//	{
	//		return DebugData->GetSnapshotLengthInFrames();
	//	}
	//}

	return 1;
}

float SShowSequencerScrubPanel::GetSequenceLength() const
{
	//if (DoesSyncViewport())
	//{
	//	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//	return PreviewInstance->GetLength();
	//}
	//else if (LockedSequence)
	//{
	//	return LockedSequence->GetPlayLength();
	//}
	//else
	//{
	//	UAnimInstance* Instance;
	//	FAnimBlueprintDebugData* DebugData;
	//	if (GetAnimBlueprintDebugData(/*out*/ Instance, /*out*/ DebugData))
	//	{
	//		return static_cast<float>(Instance->LifeTimer);
	//	}
	//}

	return 0.f;
}

bool SShowSequencerScrubPanel::DoesSyncViewport() const
{
	/*UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();

	return ((ShowSequncer == nullptr && PreviewInstance) || (ShowSequncer && PreviewInstance && PreviewInstance->GetCurrentAsset() == ShowSequncer));*/
	return false;
}

void SShowSequencerScrubPanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
}

class UAnimSingleNodeInstance* SShowSequencerScrubPanel::GetPreviewInstance() const
{
	return nullptr;
}

float SShowSequencerScrubPanel::GetScrubValue() const
{
	//if (DoesSyncViewport())
	//{
	//	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//	if (PreviewInstance)
	//	{
	//		return PreviewInstance->GetCurrentTime();
	//	}
	//}
	//else
	//{
	//	UAnimInstance* Instance;
	//	FAnimBlueprintDebugData* DebugData;
	//	if (GetAnimBlueprintDebugData(/*out*/ Instance, /*out*/ DebugData))
	//	{
	//		return static_cast<float>(Instance->CurrentLifeTimerScrubPosition);
	//	}
	//}

	return 0.f;
}

void SShowSequencerScrubPanel::ReplaceLockedSequence(UShowSequencer* InShowSequencer)
{
	EditShowSequencer = InShowSequencer;
}

UAnimInstance* SShowSequencerScrubPanel::GetAnimInstanceWithBlueprint() const
{
	/*if (UDebugSkelMeshComponent* DebugComponent = GetPreviewScene()->GetPreviewMeshComponent())
	{
		UAnimInstance* Instance = DebugComponent->GetAnimInstance();

		if ((Instance != nullptr) && (Instance->GetClass()->ClassGeneratedBy != nullptr))
		{
			return Instance;
		}
	}*/

	return nullptr;
}

bool SShowSequencerScrubPanel::GetAnimBlueprintDebugData(UAnimInstance*& Instance, FAnimBlueprintDebugData*& DebugInfo) const
{
	//Instance = GetAnimInstanceWithBlueprint();

	//if (Instance != nullptr)
	//{
	//	// Avoid updating the instance if we're replaying the past
	//	if (UAnimBlueprintGeneratedClass* AnimBlueprintClass = Cast<UAnimBlueprintGeneratedClass>(Instance->GetClass()))
	//	{
	//		if (UAnimBlueprint* Blueprint = Cast<UAnimBlueprint>(AnimBlueprintClass->ClassGeneratedBy))
	//		{
	//			if (Blueprint->GetObjectBeingDebugged() == Instance)
	//			{
	//				DebugInfo = &(AnimBlueprintClass->GetAnimBlueprintDebugData());
	//				return true;
	//			}
	//		}
	//	}
	//}

	return false;
}

void SShowSequencerScrubPanel::OnCropAnimSequence(bool bFromStart, float CurrentTime)
{
	//UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//if (PreviewInstance)
	//{
	//	float Length = PreviewInstance->GetLength();
	//	if (PreviewInstance->GetCurrentAsset())
	//	{
	//		UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset());
	//		if (AnimSequence)
	//		{
	//			const FScopedTransaction Transaction(LOCTEXT("CropAnimSequence", "Crop Animation Sequence"));

	//			//Call modify to restore slider position
	//			PreviewInstance->Modify();

	//			//Call modify to restore anim sequence current state
	//			AnimSequence->Modify();


	//			const float TrimStart = bFromStart ? 0.f : CurrentTime;
	//			const float TrimEnd = bFromStart ? CurrentTime : AnimSequence->GetPlayLength();

	//			// Trim off the user-selected part of the raw anim data.
	//			UE::Anim::AnimationData::Trim(AnimSequence, TrimStart, TrimEnd);

	//			//Resetting slider position to the first frame
	//			PreviewInstance->SetPosition(0.0f, false);

	//			OnSetInputViewRange.ExecuteIfBound(0, AnimSequence->GetPlayLength());
	//		}
	//	}
	//}
}

void SShowSequencerScrubPanel::OnAppendAnimSequence(bool bFromStart, int32 NumOfFrames)
{
	//UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//if (PreviewInstance && PreviewInstance->GetCurrentAsset())
	//{
	//	UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset());
	//	if (AnimSequence)
	//	{
	//		const FScopedTransaction Transaction(LOCTEXT("InsertAnimSequence", "Insert Animation Sequence"));

	//		//Call modify to restore slider position
	//		PreviewInstance->Modify();

	//		//Call modify to restore anim sequence current state
	//		AnimSequence->Modify();

	//		// Crop the raw anim data.
	//		int32 StartFrame = (bFromStart) ? 0 : AnimSequence->GetDataModel()->GetNumberOfFrames() - 1;
	//		int32 EndFrame = StartFrame + NumOfFrames;
	//		int32 CopyFrame = StartFrame;
	//		UE::Anim::AnimationData::DuplicateKeys(AnimSequence, StartFrame, NumOfFrames, CopyFrame);

	//		OnSetInputViewRange.ExecuteIfBound(0, AnimSequence->GetPlayLength());
	//	}
	//}
}

void SShowSequencerScrubPanel::OnInsertAnimSequence(bool bBefore, int32 CurrentFrame)
{
	//UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//if (PreviewInstance && PreviewInstance->GetCurrentAsset())
	//{
	//	UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset());
	//	if (AnimSequence)
	//	{
	//		const FScopedTransaction Transaction(LOCTEXT("InsertAnimSequence", "Insert Animation Sequence"));

	//		//Call modify to restore slider position
	//		PreviewInstance->Modify();

	//		//Call modify to restore anim sequence current state
	//		AnimSequence->Modify();

	//		// Duplicate specified key
	//		const int32 StartFrame = (bBefore) ? CurrentFrame : CurrentFrame + 1;
	//		UE::Anim::AnimationData::DuplicateKeys(AnimSequence, StartFrame, 1, CurrentFrame);

	//		OnSetInputViewRange.ExecuteIfBound(0, AnimSequence->GetPlayLength());
	//	}
	//}
}

void SShowSequencerScrubPanel::OnReZeroAnimSequence(int32 FrameIndex)
{
	//UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	//if (PreviewInstance)
	//{
	//	UDebugSkelMeshComponent* PreviewSkelComp = GetPreviewScene()->GetPreviewMeshComponent();

	//	if (PreviewInstance->GetCurrentAsset() && PreviewSkelComp)
	//	{
	//		if (UAnimSequence* AnimSequence = Cast<UAnimSequence>(PreviewInstance->GetCurrentAsset()))
	//		{
	//			if (const USkeleton* Skeleton = AnimSequence->GetSkeleton())
	//			{
	//				const FName RootBoneName = Skeleton->GetReferenceSkeleton().GetBoneName(0);

	//				if (AnimSequence->GetDataModel()->IsValidBoneTrackName(RootBoneName))
	//				{
	//					TArray<FVector3f> PosKeys;
	//					TArray<FQuat4f> RotKeys;
	//					TArray<FVector3f> ScaleKeys;

	//					TArray<FTransform> BoneTransforms;
	//					AnimSequence->GetDataModel()->GetBoneTrackTransforms(RootBoneName, BoneTransforms);

	//					PosKeys.SetNum(BoneTransforms.Num());
	//					RotKeys.SetNum(BoneTransforms.Num());
	//					ScaleKeys.SetNum(BoneTransforms.Num());

	//					// Find vector that would translate current root bone location onto origin.
	//					FVector FrameTransform = FVector::ZeroVector;
	//					if (FrameIndex == INDEX_NONE)
	//					{
	//						// Use current transform
	//						FrameTransform = PreviewSkelComp->GetComponentSpaceTransforms()[0].GetLocation();
	//					}
	//					else if (BoneTransforms.IsValidIndex(FrameIndex))
	//					{
	//						// Use transform at frame
	//						FrameTransform = BoneTransforms[FrameIndex].GetLocation();
	//					}

	//					FVector ApplyTranslation = -1.f * FrameTransform;

	//					// Convert into world space
	//					const FVector WorldApplyTranslation = PreviewSkelComp->GetComponentTransform().TransformVector(ApplyTranslation);
	//					ApplyTranslation = PreviewSkelComp->GetComponentTransform().InverseTransformVector(WorldApplyTranslation);

	//					for (int32 KeyIndex = 0; KeyIndex < BoneTransforms.Num(); KeyIndex++)
	//					{
	//						PosKeys[KeyIndex] = FVector3f(BoneTransforms[KeyIndex].GetLocation() + ApplyTranslation);
	//						RotKeys[KeyIndex] = FQuat4f(BoneTransforms[KeyIndex].GetRotation());
	//						ScaleKeys[KeyIndex] = FVector3f(BoneTransforms[KeyIndex].GetScale3D());
	//					}

	//					IAnimationDataController& Controller = AnimSequence->GetController();
	//					Controller.SetBoneTrackKeys(RootBoneName, PosKeys, RotKeys, ScaleKeys);
	//				}
	//			}
	//		}
	//	}
	//}
}

bool SShowSequencerScrubPanel::GetDisplayDrag() const
{
	UAnimSingleNodeInstance* PreviewInstance = GetPreviewInstance();
	if (PreviewInstance && PreviewInstance->GetCurrentAsset())
	{
		return true;
	}

	return false;
}
