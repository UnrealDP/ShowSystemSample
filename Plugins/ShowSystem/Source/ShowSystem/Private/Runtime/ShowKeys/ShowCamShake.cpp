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
    switch (CamShakeKeyPtr->CameraShakePattern)
    {
        case ECameraShakePattern::PerlinNoise:
        {
            if (CamShakeKeyPtr->PatternData.IsValid() && CamShakeKeyPtr->PatternData.GetScriptStruct() == FShowPerlinNoiseCameraShake::StaticStruct())
            {
                const FShowPerlinNoiseCameraShake& PerlinShakeData = CamShakeKeyPtr->PatternData.Get<FShowPerlinNoiseCameraShake>();
                return PerlinShakeData.Duration / ShowKey->PlayRate;
            }
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            if (CamShakeKeyPtr->PatternData.IsValid() && CamShakeKeyPtr->PatternData.GetScriptStruct() == FShowWaveOscCamShake::StaticStruct())
            {
                const FShowWaveOscCamShake& ShowWaveOscShakeData = CamShakeKeyPtr->PatternData.Get<FShowWaveOscCamShake>();
                return ShowWaveOscShakeData.Duration / ShowKey->PlayRate;
            }
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            if (CamShakeKeyPtr->PatternData.IsValid() && CamShakeKeyPtr->PatternData.GetScriptStruct() == FShowSequenceCameraShake::StaticStruct())
            {
                const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

                if (!SequenceShakeData.Sequence)
                {
                    return 0.0f;
                }

                if (SequenceShakeData.bRandomSegment)
                {
                    return SequenceShakeData.RandomSegmentDuration / ShowKey->PlayRate;
                }
                else if (UMovieScene* MovieScene = SequenceShakeData.Sequence->GetMovieScene())
                {
                    FFrameRate FrameRate = MovieScene->GetTickResolution();
                    FFrameNumber StartFrame = MovieScene->GetPlaybackRange().GetLowerBoundValue();
                    FFrameNumber EndFrame = MovieScene->GetPlaybackRange().GetUpperBoundValue();

                    return FrameRate.AsSeconds(EndFrame - StartFrame) / ShowKey->PlayRate;
                }
            }
            break;
        }
        default:
            break;
    }

    return 0.0f;
}

void UShowCamShake::Initialize()
{
    checkf(ShowKey, TEXT("UShowCamShake::Initialize ShowKey is invalid"));
    
    CamShakeKeyPtr = static_cast<const FShowCamShakeKey*>(ShowKey);
    checkf(CamShakeKeyPtr, TEXT("UShowCamShake::Initialize CamShakeKeyPtr is invalid [ %d ]"), static_cast<int>(ShowKey->KeyType));

#if WITH_EDITOR
    InitBackupPatternData();
#endif
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

#if WITH_EDITOR
    BackupPatternData.Empty();
#endif
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

    if (!CameraShakeInstance)
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    float TimeScale = ShowKey->PlayRate * CachedTimeScale;
    const UScriptStruct* ScriptStruct = CamShakeKeyPtr->PatternData.GetScriptStruct();
    switch (CamShakeKeyPtr->CameraShakePattern)
    {
        case ECameraShakePattern::PerlinNoise:
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
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            const FShowWaveOscCamShake& ShowWaveOscData = CamShakeKeyPtr->PatternData.Get<FShowWaveOscCamShake>();

            UShowWaveOscCamShakePattern* ShakePattern = NewObject<UShowWaveOscCamShakePattern>(CameraShakeInstance, ShowWaveOscData.CameraShakePattern);

            ShakePattern->Duration = ShowWaveOscData.Duration / TimeScale;
            ShakePattern->BlendInTime = ShowWaveOscData.BlendInTime;
            ShakePattern->BlendOutTime = ShowWaveOscData.BlendOutTime;
            ShakePattern->LocationAmplitudeMultiplier = ShowWaveOscData.LocationAmplitudeMultiplier;
            ShakePattern->LocationFrequencyMultiplier = ShowWaveOscData.LocationFrequencyMultiplier * TimeScale;
            ShakePattern->X = ShowWaveOscData.X;
            ShakePattern->Y = ShowWaveOscData.Y;
            ShakePattern->Z = ShowWaveOscData.Z;
            ShakePattern->RotationAmplitudeMultiplier = ShowWaveOscData.RotationAmplitudeMultiplier;
            ShakePattern->RotationFrequencyMultiplier = ShowWaveOscData.RotationFrequencyMultiplier * TimeScale;
            ShakePattern->Pitch = ShowWaveOscData.Pitch;
            ShakePattern->Yaw = ShowWaveOscData.Yaw;
            ShakePattern->Roll = ShowWaveOscData.Roll;
            ShakePattern->FOV = ShowWaveOscData.FOV;

            CameraShakeInstance->SetRootShakePattern(ShakePattern);
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

            UShowSequenceCamShakePattern* ShakePattern = NewObject<UShowSequenceCamShakePattern>(CameraShakeInstance, SequenceShakeData.CameraShakePattern);

            ShakePattern->Sequence = SequenceShakeData.Sequence;
            ShakePattern->PlayRate = TimeScale;
            ShakePattern->Scale = SequenceShakeData.Scale;
            ShakePattern->BlendInTime = SequenceShakeData.BlendInTime;
            ShakePattern->BlendOutTime = SequenceShakeData.BlendOutTime;
            ShakePattern->RandomSegmentDuration = SequenceShakeData.RandomSegmentDuration;
            ShakePattern->bRandomSegment = SequenceShakeData.bRandomSegment;

            CameraShakeInstance->SetRootShakePattern(ShakePattern);
            break;
        }
        default:
            break;
    }

    // Shake를 수동으로 시작
    FCameraShakeBaseStartParams StartParams;
    StartParams.CameraManager = PlayerController->PlayerCameraManager;
    StartParams.Scale = 1.0f;
    StartParams.PlaySpace = CamShakeKeyPtr->PlaySpace;
    StartParams.UserPlaySpaceRot = CamShakeKeyPtr->UserPlaySpaceRot;

    CameraShakeInstance->StartShake(StartParams);
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
    if (!CameraShakeInstance)
    {
        return;
    }
    
    UCameraShakePattern* ShakePattern = CameraShakeInstance->GetRootShakePattern();
    if (!ShakePattern)
    {
        return;
    }

    float TimeScale = ShowKey->PlayRate * FinalTimeScale;    
    switch (CamShakeKeyPtr->CameraShakePattern)
    {
        case ECameraShakePattern::PerlinNoise:
        {
            if (UShowPerlinNoiseCamShakePattern* PerlinPattern = Cast<UShowPerlinNoiseCamShakePattern>(ShakePattern))
            {
                const FShowPerlinNoiseCameraShake& PerlinShakeData = CamShakeKeyPtr->PatternData.Get<FShowPerlinNoiseCameraShake>();

                PerlinPattern->LocationFrequencyMultiplier = PerlinShakeData.LocationFrequencyMultiplier * TimeScale;
                PerlinPattern->RotationFrequencyMultiplier = PerlinShakeData.RotationFrequencyMultiplier * TimeScale;
                PerlinPattern->UpdateDuration(PerlinShakeData.Duration / TimeScale);
            }
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            if (UShowWaveOscCamShakePattern* PerlinPattern = Cast<UShowWaveOscCamShakePattern>(ShakePattern))
            {
                const FShowWaveOscCamShake& ShowWaveOscShakeData = CamShakeKeyPtr->PatternData.Get<FShowWaveOscCamShake>();

                PerlinPattern->LocationFrequencyMultiplier = ShowWaveOscShakeData.LocationFrequencyMultiplier * TimeScale;
                PerlinPattern->RotationFrequencyMultiplier = ShowWaveOscShakeData.RotationFrequencyMultiplier * TimeScale;
                PerlinPattern->UpdateDuration(ShowWaveOscShakeData.Duration / TimeScale);
            }
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            if (UShowSequenceCamShakePattern* SequencePattern = Cast<UShowSequenceCamShakePattern>(ShakePattern))
            {
                const FShowSequenceCameraShake& SequenceShakeData = CamShakeKeyPtr->PatternData.Get<FShowSequenceCameraShake>();

                SequencePattern->PlayRate = TimeScale;
                if (SequencePattern->bRandomSegment)
                {
                    SequencePattern->RandomSegmentDuration = SequenceShakeData.RandomSegmentDuration / TimeScale;
                }
                SequencePattern->UpdatePlayRate(TimeScale);
            }
            break;
        }
        default:
            break;
    }
}