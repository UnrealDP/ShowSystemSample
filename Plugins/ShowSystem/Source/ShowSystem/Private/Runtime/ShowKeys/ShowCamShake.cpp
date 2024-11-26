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
    if (!CamShakeKeyPtr->PatternData.IsValid())
    {
        ReConstructPatternData();
    }
    
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
#if WITH_EDITOR
        if (GEditor->bIsSimulatingInEditor)
        {
            InitialLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
            InitialRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
        }
#endif

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

            ShakePattern->Duration = PerlinShakeData.Duration;
            //ShakePattern->Duration = PerlinShakeData.Duration / TimeScale;
            ShakePattern->BlendInTime = PerlinShakeData.BlendInTime;
            ShakePattern->BlendOutTime = PerlinShakeData.BlendOutTime;
            ShakePattern->LocationAmplitudeMultiplier = PerlinShakeData.LocationAmplitudeMultiplier;
            ShakePattern->LocationFrequencyMultiplier = PerlinShakeData.LocationFrequencyMultiplier;
            //ShakePattern->LocationFrequencyMultiplier = PerlinShakeData.LocationFrequencyMultiplier * TimeScale;
            ShakePattern->X = PerlinShakeData.X;
            ShakePattern->Y = PerlinShakeData.Y;
            ShakePattern->Z = PerlinShakeData.Z;
            ShakePattern->RotationAmplitudeMultiplier = PerlinShakeData.RotationAmplitudeMultiplier;
            ShakePattern->RotationFrequencyMultiplier = PerlinShakeData.RotationFrequencyMultiplier;
            //ShakePattern->RotationFrequencyMultiplier = PerlinShakeData.RotationFrequencyMultiplier * TimeScale;
            ShakePattern->Pitch = PerlinShakeData.Pitch;
            ShakePattern->Yaw = PerlinShakeData.Yaw;
            ShakePattern->Roll = PerlinShakeData.Roll;
            ShakePattern->FOV = PerlinShakeData.FOV;

            ShakePattern->UpdatePlayRate(TimeScale);

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
    if (PlayerController)
    {
        FCameraShakeBaseStartParams StartParams;
        StartParams.CameraManager = PlayerController->PlayerCameraManager;
        StartParams.Scale = 1.0f;
        StartParams.PlaySpace = CamShakeKeyPtr->PlaySpace;
        StartParams.UserPlaySpaceRot = CamShakeKeyPtr->UserPlaySpaceRot;

        CameraShakeInstance->StartShake(StartParams);
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

#if WITH_EDITOR
        UpdateSimulatingInEditor();
#endif
    }
}
#if WITH_EDITOR
void UShowCamShake::UpdateSimulatingInEditor()
{
    if (GEditor->bIsSimulatingInEditor)
    {
        APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
        if (PlayerController && PlayerController->PlayerCameraManager)
        {
            FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
            if (EditorViewportClient)
            {
                // 현재 카메라 위치와 회전 가져오기
                FVector CurrentLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
                FRotator CurrentRotation = PlayerController->PlayerCameraManager->GetCameraRotation();

                // 변화량 계산
                FVector DeltaLocation = CurrentLocation - InitialLocation;
                FRotator DeltaRotation = CurrentRotation - InitialRotation;

                // 뷰포트 카메라에 변화량 적용
                FVector NewViewportLocation = EditorViewportClient->GetViewLocation() + DeltaLocation;
                FRotator NewViewportRotation = EditorViewportClient->GetViewRotation() + DeltaRotation;

                EditorViewportClient->SetViewLocation(NewViewportLocation);
                EditorViewportClient->SetViewRotation(NewViewportRotation);
                EditorViewportClient->Invalidate(); // 뷰포트 갱신

                // 현재 값을 초기값으로 갱신
                InitialLocation = CurrentLocation;
                InitialRotation = CurrentRotation;
            }
        }
    }
}
#endif
void UShowCamShake::Pause()
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

    switch (CamShakeKeyPtr->CameraShakePattern)
    {
        case ECameraShakePattern::PerlinNoise:
        {
            if (UShowPerlinNoiseCamShakePattern* PerlinPattern = Cast<UShowPerlinNoiseCamShakePattern>(ShakePattern))
            {
                PerlinPattern->Pause();
            }
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            if (UShowWaveOscCamShakePattern* WaveOscCamPattern = Cast<UShowWaveOscCamShakePattern>(ShakePattern))
            {
                WaveOscCamPattern->Pause();
            }
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            if (UShowSequenceCamShakePattern* SequencePattern = Cast<UShowSequenceCamShakePattern>(ShakePattern))
            {
                SequencePattern->Pause();
            }
            break;
        }
        default:
            break;
    }
}

void UShowCamShake::UnPause()
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

    switch (CamShakeKeyPtr->CameraShakePattern)
    {
        case ECameraShakePattern::PerlinNoise:
        {
            if (UShowPerlinNoiseCamShakePattern* PerlinPattern = Cast<UShowPerlinNoiseCamShakePattern>(ShakePattern))
            {
                PerlinPattern->UnPause();
            }
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            if (UShowWaveOscCamShakePattern* WaveOscCamPattern = Cast<UShowWaveOscCamShakePattern>(ShakePattern))
            {
                WaveOscCamPattern->UnPause();
            }
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            if (UShowSequenceCamShakePattern* SequencePattern = Cast<UShowSequenceCamShakePattern>(ShakePattern))
            {
                SequencePattern->UnPause();
            }
            break;
        }
        default:
            break;
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
                PerlinPattern->UpdatePlayRate(TimeScale);
            }
            break;
        }
        case ECameraShakePattern::WaveOscillator:
        {
            if (UShowWaveOscCamShakePattern* WaveOscCamPattern = Cast<UShowWaveOscCamShakePattern>(ShakePattern))
            {
                WaveOscCamPattern->UpdatePlayRate(TimeScale);
            }
            break;
        }
        case ECameraShakePattern::Sequence:
        {
            if (UShowSequenceCamShakePattern* SequencePattern = Cast<UShowSequenceCamShakePattern>(ShakePattern))
            {
                SequencePattern->UpdatePlayRate(TimeScale);
            }
            break;
        }
        default:
            break;
    }
}

void UShowCamShake::SetPassedTime(float InTime)
{
    if (!CameraShakeInstance)
    {
        Play();
    }
    if (!CameraShakeInstance)
    {
        return;
    }

    FMinimalViewInfo POV;
    CameraShakeInstance->ScrubAndApplyCameraShake(InTime, 1.f, POV);
#if WITH_EDITOR
    UpdateSimulatingInEditor();
#endif
}

