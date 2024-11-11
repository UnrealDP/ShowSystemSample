// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowCamShake.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "CameraAnimationSequence.h"
#include "MovieScene.h"
#include "Runtime//Misc/ShowCameraShakeBase.h"

FString UShowCamShake::GetTitle()
{
	return "ShowCamShake";
}

float UShowCamShake::GetLength()
{
    const UScriptStruct* ScriptStruct = CamShakeKeyPtr->PatternData.GetScriptStruct();
    if (ScriptStruct == FShowPerlinNoiseCameraShake::StaticStruct())
    {
        const FShowPerlinNoiseCameraShake& PerlinShakeData = CamShakeKeyPtr->PatternData.Get<FShowPerlinNoiseCameraShake>();
        return PerlinShakeData.Duration / ShowKey->PlayRate;
    }
    else if (ScriptStruct == FShowSequenceCameraShake::StaticStruct())
    {
        const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

        if (!SequenceShakeData.Sequence)
        {
            return 0.0f;
        }

        if (UMovieScene* MovieScene = SequenceShakeData.Sequence->GetMovieScene())
        {
            FFrameRate FrameRate = MovieScene->GetTickResolution();
            FFrameNumber StartFrame = MovieScene->GetPlaybackRange().GetLowerBoundValue();
            FFrameNumber EndFrame = MovieScene->GetPlaybackRange().GetUpperBoundValue();

            return FrameRate.AsSeconds(EndFrame - StartFrame) / ShowKey->PlayRate;
		}            

        return 0.0f;
    }

    return 0.0f;
}

void UShowCamShake::Initialize()
{
    checkf(ShowKey, TEXT("UShowCamShake::Initialize ShowKey is invalid"));
    
    CamShakeKeyPtr = static_cast<const FShowCamShakeKey*>(ShowKey);
    checkf(CamShakeKeyPtr, TEXT("UShowCamShake::Initialize CamShakeKeyPtr is invalid [ %d ]"), static_cast<int>(ShowKey->KeyType));
}

void UShowCamShake::Dispose()
{
    if (CameraShakeInstance)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            if (PlayerController && PlayerController->PlayerCameraManager)
            {
                PlayerController->PlayerCameraManager->StopCameraShake(CameraShakeInstance, true);
                CameraShakeInstance->ConditionalBeginDestroy();
                CameraShakeInstance = nullptr;
            }
        }
        else
        {
            ShowKeyState = EShowKeyState::ShowKey_End;
        }
    }
}

void UShowCamShake::Play()
{
    checkf(!CameraShakeInstance, TEXT("UShowCamShake::Play CameraShakeInstance is not null"));

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        if (PlayerController && PlayerController->PlayerCameraManager)
        {
            CameraShakeInstance = NewObject<UShowCameraShakeBase>(PlayerController->PlayerCameraManager);

            CameraShakeInstance = PlayerController->PlayerCameraManager->StartCameraShake(
                UShowCameraShakeBase::StaticClass(),
                1.0f,
                CamShakeKeyPtr->PlaySpace,
                CamShakeKeyPtr->UserPlaySpaceRot);

            CameraShakeInstance->bSingleInstance = CamShakeKeyPtr->bSingleInstance;
            CameraShakeInstance->TeardownShake();
        }
    }
    else
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
    }

    if (CameraShakeInstance)
    {
        float TimeScale = ShowKey->PlayRate * CachedTimeScale;
        const UScriptStruct* ScriptStruct = CamShakeKeyPtr->PatternData.GetScriptStruct();
        if (ScriptStruct == FShowPerlinNoiseCameraShake::StaticStruct())
        {
            const FShowPerlinNoiseCameraShake& PerlinShakeData = CamShakeKeyPtr->PatternData.Get<FShowPerlinNoiseCameraShake>();

            UShowPerlinNoiseCamShakePattern* ShakePattern = NewObject<UShowPerlinNoiseCamShakePattern>(CameraShakeInstance, PerlinShakeData.CameraShakePattern);

            ShakePattern->Duration = PerlinShakeData.Duration / TimeScale;
            ShakePattern->BlendInTime = PerlinShakeData.BlendInTime;
            ShakePattern->BlendOutTime = PerlinShakeData.BlendOutTime;
            ShakePattern->LocationAmplitudeMultiplier = PerlinShakeData.LocationAmplitudeMultiplier;
            ShakePattern->LocationFrequencyMultiplier = PerlinShakeData.LocationFrequencyMultiplier * TimeScale;
            ShakePattern->X = PerlinShakeData.X;
            ShakePattern->Y = PerlinShakeData.Y;
            ShakePattern->Z = PerlinShakeData.Z;
            ShakePattern->RotationAmplitudeMultiplier = PerlinShakeData.RotationAmplitudeMultiplier;
            ShakePattern->RotationFrequencyMultiplier = PerlinShakeData.RotationFrequencyMultiplier * TimeScale;
            ShakePattern->Pitch = PerlinShakeData.Pitch;
            ShakePattern->Yaw = PerlinShakeData.Yaw;
            ShakePattern->Roll = PerlinShakeData.Roll;
            ShakePattern->FOV = PerlinShakeData.FOV;

            CameraShakeInstance->SetRootShakePattern(ShakePattern);
        }
        else if (ScriptStruct == FShowSequenceCameraShake::StaticStruct())
        {
            const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

            UShowSequenceCamShakePattern* ShakePattern = NewObject<UShowSequenceCamShakePattern>(CameraShakeInstance, SequenceShakeData.CameraShakePattern);

            ShakePattern->Sequence = SequenceShakeData.Sequence;
            ShakePattern->PlayRate = SequenceShakeData.PlayRate * TimeScale;
            ShakePattern->Scale = SequenceShakeData.Scale;
            ShakePattern->BlendInTime = SequenceShakeData.BlendInTime;
            ShakePattern->BlendOutTime = SequenceShakeData.BlendOutTime;
            ShakePattern->RandomSegmentDuration = SequenceShakeData.RandomSegmentDuration;
            ShakePattern->bRandomSegment = SequenceShakeData.bRandomSegment;

            CameraShakeInstance->SetRootShakePattern(ShakePattern);
        }

        // Remember the various settings for this run.
    // Note that the camera manager can be null, for example in unit tests.
        //CameraManager = Params.CameraManager;
        //ShakeScale = Params.Scale;
        //PlaySpace = Params.PlaySpace;
        //UserPlaySpaceMatrix = (Params.PlaySpace == ECameraShakePlaySpace::UserDefined) ?
        //    FRotationMatrix(Params.UserPlaySpaceRot) : FRotationMatrix::Identity;

        //const bool bIsRestarting = bIsActive;
        //bIsActive = true;

        //// Let the root pattern initialize itself.
        //if (RootShakePattern)
        //{
        //    FCameraShakePatternStartParams StartParams;
        //    StartParams.bIsRestarting = bIsRestarting;
        //    StartParams.bOverrideDuration = Params.DurationOverride.IsSet();
        //    StartParams.DurationOverride = Params.DurationOverride.Get(0.f);
        //    RootShakePattern->StartShakePattern(StartParams);
        //}

        // Shake를 수동으로 시작
        FCameraShakeBaseStartParams StartParams;
        StartParams.CameraManager = PlayerController->PlayerCameraManager;
        StartParams.Scale = 1.0f;
        StartParams.PlaySpace = CamShakeKeyPtr->PlaySpace;
        StartParams.UserPlaySpaceRot = CamShakeKeyPtr->UserPlaySpaceRot;

        CameraShakeInstance->StartShake(StartParams);
    }
    else
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
    }
}

void UShowCamShake::Reset()
{
    Dispose();
}

void UShowCamShake::Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime)
{
    if (ShowKeyState != EShowKeyState::ShowKey_Playing)
    {
        return;
    }

    if (CameraShakeInstance)
    {
        if (CameraShakeInstance->IsFinished())
        {
            ShowKeyState = EShowKeyState::ShowKey_End;
        }
    }
}

void UShowCamShake::ApplyTimeScale(float FinalTimeScale)
{
    if (CameraShakeInstance)
    {
        UCameraShakePattern* ShakePattern = CameraShakeInstance->GetRootShakePattern();

        if (ShakePattern)
        {
            float TimeScale = ShowKey->PlayRate * FinalTimeScale;
            if (UShowPerlinNoiseCamShakePattern* PerlinPattern = Cast<UShowPerlinNoiseCamShakePattern>(ShakePattern))
            {
                const FShowPerlinNoiseCameraShake& PerlinShakeData = CamShakeKeyPtr->PatternData.Get<FShowPerlinNoiseCameraShake>();

                PerlinPattern->LocationFrequencyMultiplier = PerlinShakeData.LocationFrequencyMultiplier * TimeScale;
                PerlinPattern->RotationFrequencyMultiplier = PerlinShakeData.RotationFrequencyMultiplier * TimeScale;
                PerlinPattern->UpdateDuration(PerlinShakeData.Duration / TimeScale);

                UE_LOG(LogTemp, Warning, TEXT("PerlinPattern->LocationFrequencyMultiplier : %f"), PerlinPattern->LocationFrequencyMultiplier);
            }
            else if (UShowSequenceCamShakePattern* SequencePattern = Cast<UShowSequenceCamShakePattern>(ShakePattern))
            {
                const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

                SequencePattern->PlayRate = SequenceShakeData.PlayRate * TimeScale;
                if (SequencePattern->bRandomSegment)
                {
                    SequencePattern->RandomSegmentDuration = SequenceShakeData.RandomSegmentDuration / TimeScale;
                }
                //SequencePattern->UpdateDuration(SequencePattern->Duration / TimeScale);
            }
        }
    }
}