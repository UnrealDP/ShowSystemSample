// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowCamSequence.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

FString UShowCamSequence::GetTitle()
{
    return "ShowCamSequence";
}

void UShowCamSequence::Initialize()
{
    checkf(ShowKey, TEXT("UShowCamSequence::Initialize ShowKey is invalid"));

    ShowCamSequenceKeyPtr = static_cast<const FShowCamSequenceKey*>(ShowKey);
    checkf(ShowCamSequenceKeyPtr, TEXT("UShowCamSequence::Initialize ShowCamSequenceKeyPtr is invalid [ %d ]"), static_cast<int>(ShowKey->KeyType));
}

float UShowCamSequence::GetLength()
{
    float Length = 0.0f;
    for (FCameraPathPoint CameraPathPoint : ShowCamSequenceKeyPtr->PathPoints)
    {
        Length += CameraPathPoint.Duration;
    }
    
    if (ShowCamSequenceKeyPtr->bReturnToStartPosition)
	{
        Length += ShowCamSequenceKeyPtr->FadeOutBlendTime;
	}

    return Length / ShowKey->PlayRate;
}

void UShowCamSequence::Dispose()
{
    ShowCamSequenceKeyPtr = nullptr;
    State = ECameraSequenceState::Wait;
    CurrentBlendTime = 0.0f;
    CurrentPointIndex = 0;
}

void UShowCamSequence::Reset()
{
    Dispose();
    Initialize();
}

void UShowCamSequence::Play()
{
    AActor* OwnerActor = GetShowOwner();
    checkf(OwnerActor, TEXT("UShowCamSequence::Tick OwnerActor is invalid"));

    // 최소 한 개의 포인트가 필요
    if (ShowCamSequenceKeyPtr->PathPoints.Num() < 1)
    {
        UE_LOG(LogTemp, Error, TEXT("UShowCamSequence::Play PathPoints is empty"));
        return;
    }
    
    PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        APawn* ControlledPawn = PlayerController->GetPawn();
        if (ControlledPawn)
        {
            CameraComponent = ControlledPawn->FindComponentByClass<UCameraComponent>();
            SpringArmComponent = ControlledPawn->FindComponentByClass<USpringArmComponent>();

            if (SpringArmComponent && CameraComponent)
            {
                // SpringArm의 소켓 위치를 가져와서 카메라 위치와 비교하여 오프셋 계산
                FTransform SocketTransform = SpringArmComponent->GetSocketTransform(NAME_None, RTS_World);
                FVector SocketWorldLocation = SocketTransform.GetLocation();

                // 카메라 위치와 소켓 위치의 오프셋 저장
                InitialCameraOffset = CameraComponent->GetComponentLocation() - SocketWorldLocation;
                InitialFOV = PlayerController->PlayerCameraManager->GetFOVAngle();
                InitialFOVForBlend = InitialFOV;

                // 카메라의 현재 LookAt 위치를 계산하고 소켓 기준으로 오프셋 저장
                FVector CurrentLookAt = GetCameraLookAt();
                InitialLookAtOffset = CurrentLookAt - SocketWorldLocation;

                // SpringArm 비활성화
                SpringArmComponent->Activate(false);

                // 시작 위치를 Actor의 상대 좌표로 저장
                FVector ActorWorldPosition = OwnerActor->GetActorLocation();
                InitialCameraRelativePositionForBlend = GetCameraLocation() - ActorWorldPosition;
                InitialLookAtRelativeTargetForBlend = GetCameraLookAt() - ActorWorldPosition;

                InitialLocalCharacterLocation = OwnerActor->GetActorTransform().InverseTransformPosition(SpringArmComponent->GetComponentLocation());
                InitialLocalCharacterRotation = OwnerActor->GetActorTransform().InverseTransformRotation(SpringArmComponent->GetComponentRotation().Quaternion()).Rotator();
            }

            PlayerController->SetIgnoreLookInput(true);
        }
    }

    State = ECameraSequenceState::Playing;
    CurrentBlendTime = 0.0f;
    CurrentPointIndex = 0;
}

void UShowCamSequence::Tick(float DeltaTime, float BasePassedTime)
{
    if (State == ECameraSequenceState::Wait)
	{
		return;
	}

    AActor* OwnerActor = GetShowOwner();
    if (!OwnerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("UShowCamSequence::Tick OwnerActor is null."));
        return;
    }

    CurrentBlendTime += DeltaTime;

    switch (State)
    {
        case ECameraSequenceState::Playing:
        {
            PathPointPlay(OwnerActor);
            break;
        }
        case ECameraSequenceState::ReturningToStart:
        {
            PathPointReturningToStart(OwnerActor);
            break;
        }
        case ECameraSequenceState::End:
            if (SpringArmComponent)
            {
                PlayerController->SetIgnoreLookInput(false);
                //ApplyCharacterOffsetToSpringArm();
                SpringArmComponent->Activate(true);
            }
            ShowKeyState = EShowKeyState::ShowKey_End;
            return;

        default:
            ShowKeyState = EShowKeyState::ShowKey_End;
            return;
    }
}

void UShowCamSequence::PathPointPlay(AActor* OwnerActor)
{
    const FCameraPathPoint& ArrivalPoint = ShowCamSequenceKeyPtr->PathPoints[CurrentPointIndex];

    // Actor의 현재 월드 위치 가져오기
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();

    // 보간 계산
    float CurrentDuration = ArrivalPoint.Duration;
    float Alpha = FMath::Clamp(CurrentBlendTime / CurrentDuration, 0.0f, 1.0f);

    // Actor의 현재 위치를 기반으로 월드 좌표 계산
    FVector ArrivalPositionWorld = ActorWorldPosition + ArrivalPoint.Position;
    FVector InitialPositionWorld = ActorWorldPosition + InitialCameraRelativePositionForBlend;
    FVector InterpolatedPosition = FMath::Lerp(InitialPositionWorld, ArrivalPositionWorld, Alpha);

    FVector ArrivalLookAtWorld = ActorWorldPosition + ArrivalPoint.LookAtTarget.GetSafeNormal() * 1000.0f;
    FVector InitialLookAtWorld = ActorWorldPosition + InitialLookAtRelativeTargetForBlend.GetSafeNormal() * 1000.0f;
    FVector InterpolatedLookAt = FMath::Lerp(InitialLookAtWorld, ArrivalLookAtWorld, Alpha);
    FRotator InterpolatedRotation = (InterpolatedLookAt - InterpolatedPosition).Rotation();

    // 시간 증가 및 다음 포인트로 이동 처리
    if (CurrentBlendTime >= CurrentDuration)
    {
        FRotator EndRotation = (ArrivalLookAtWorld - InterpolatedPosition).Rotation();
        TOptional<float> InterpolatedFOV;
        if (ArrivalPoint.FieldOfView.IsSet())
        {
            InterpolatedFOV = ArrivalPoint.FieldOfView.GetValue();
        }

        ApplyCameraSettings(ArrivalPositionWorld, EndRotation, InterpolatedFOV);

        CurrentBlendTime = 0.0f;
        CurrentPointIndex++;

        // 모든 포인트를 다 이동하면 페이드 아웃으로 전환
        if (CurrentPointIndex >= ShowCamSequenceKeyPtr->PathPoints.Num())
        {
            CurrentBlendTime = 0.0f;

            if (ShowCamSequenceKeyPtr->bReturnToStartPosition)
            {
                State = ECameraSequenceState::ReturningToStart;

                FVector InitialCameraLocationForBlend = GetCameraLocation();
                InitialCameraRelativePositionForBlend = InitialCameraLocationForBlend - ActorWorldPosition;

                FVector InitialLookAtTargetForBlend = GetCameraLookAt();
                InitialLookAtRelativeTargetForBlend = InitialLookAtTargetForBlend - ActorWorldPosition;

                InitialFOVForBlend = PlayerController->PlayerCameraManager->GetFOVAngle();
            }
            else
            {
                State = ECameraSequenceState::End;
            }
        }
        else
        {
            // 새로운 PathPoint가 시작될 때마다 보간의 시작 지점을 Actor의 상대 좌표로 저장
            FVector InitialCameraLocationForBlend = GetCameraLocation();
            InitialCameraRelativePositionForBlend = InitialCameraLocationForBlend - ActorWorldPosition;

            FVector InitialLookAtTargetForBlend = GetCameraLookAt();
            InitialLookAtRelativeTargetForBlend = InitialLookAtTargetForBlend - ActorWorldPosition;

            InitialFOVForBlend = PlayerController->PlayerCameraManager->GetFOVAngle();
        }
    }
    else
    {
        // FOV 보간 계산
        TOptional<float> InterpolatedFOV;
        if (ArrivalPoint.FieldOfView.IsSet())
        {
            InterpolatedFOV = FMath::Lerp(InitialFOVForBlend, ArrivalPoint.FieldOfView.GetValue(), Alpha);
        }

        // 카메라 설정 적용
        ApplyCameraSettings(InterpolatedPosition, InterpolatedRotation, InterpolatedFOV);
    }
}

void UShowCamSequence::PathPointReturningToStart(AActor* OwnerActor)
{
    if (!SpringArmComponent)
    {
        State = ECameraSequenceState::End;
        UE_LOG(LogTemp, Error, TEXT("UShowCamSequence::PathPointReturningToStart SpringArmComponent is null."));
        return;
    }

    // Actor의 현재 월드 위치 가져오기
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();
    
    float Alpha = FMath::Clamp(CurrentBlendTime / ShowCamSequenceKeyPtr->FadeOutBlendTime, 0.0f, 1.0f);

    // Actor의 현재 위치를 기반으로 월드 좌표 계산
    FTransform CurrentSocketTransform = SpringArmComponent->GetSocketTransform(NAME_None, RTS_World);

    FVector CurrentSocketWorldLocation = CurrentSocketTransform.GetLocation();
    FVector ArrivalPositionWorld = CurrentSocketWorldLocation + InitialCameraOffset;
    FVector InitialPositionWorld = ActorWorldPosition + InitialCameraRelativePositionForBlend;
    FVector InterpolatedPosition = FMath::Lerp(InitialPositionWorld, ArrivalPositionWorld, Alpha);

    FVector ArrivalLookAtWorld = CurrentSocketWorldLocation + InitialLookAtOffset.GetSafeNormal() * 1000.0f;
    FVector InitialLookAtWorld = ActorWorldPosition + InitialLookAtRelativeTargetForBlend.GetSafeNormal() * 1000.0f;
    FVector InterpolatedLookAt = FMath::Lerp(InitialLookAtWorld, ArrivalLookAtWorld, Alpha);
    FRotator InterpolatedRotation = (InterpolatedLookAt - InterpolatedPosition).Rotation();

    // 복귀 완료 시 상태 종료
    if (CurrentBlendTime >= ShowCamSequenceKeyPtr->FadeOutBlendTime)
    {
        FRotator EndRotation = (ArrivalLookAtWorld - InterpolatedPosition).Rotation();
        TOptional<float> InterpolatedFOV = InitialFOV;
        ApplyCameraSettings(ArrivalPositionWorld, EndRotation, InterpolatedFOV);
        State = ECameraSequenceState::End;
    }
    else
    {
        // FOV 보간 계산
        TOptional<float> InterpolatedFOV;
        if (InitialFOV != InitialFOVForBlend)
        {
            InterpolatedFOV = FMath::Lerp(InitialFOVForBlend, InitialFOV, Alpha);
        }
        ApplyCameraSettings(InterpolatedPosition, InterpolatedRotation, InterpolatedFOV);
    }
}

void UShowCamSequence::ApplyCameraSettings(const FVector& NewPosition, const FRotator& NewRotation, TOptional<float>& NewFOV)
{
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        // FOV 설정
        if (NewFOV.IsSet())
        {
            PlayerController->PlayerCameraManager->SetFOV(NewFOV.GetValue());
        }

        if (CameraComponent)
        {
            CameraComponent->SetWorldLocation(NewPosition);
            CameraComponent->SetWorldRotation(NewRotation);
        }
    }
}

FVector UShowCamSequence::GetCameraLocation()
{
    if (!CameraComponent)
    {
        return FVector::ZeroVector;
    }

    return CameraComponent->GetComponentLocation();
}

FVector UShowCamSequence::GetCameraLookAt(float Distance)
{
    if (!CameraComponent)
    {
        return FVector::ZeroVector;
    }

    FVector CameraLocation = CameraComponent->GetComponentLocation();
    FVector CameraDirection = CameraComponent->GetForwardVector();
    FVector LookAtTarget = CameraLocation + (CameraDirection * Distance);
    return LookAtTarget;
}

void UShowCamSequence::ApplyCharacterOffsetToSpringArm()
{
    if (SpringArmComponent)
    {
        AActor* OwnerActor = GetShowOwner();
        if (OwnerActor)
        {
            FVector CurrentLocalCharacterLocation = OwnerActor->GetActorTransform().InverseTransformPosition(SpringArmComponent->GetComponentLocation());
            FRotator CurrentLocalCharacterRotation = OwnerActor->GetActorTransform().InverseTransformRotation(SpringArmComponent->GetComponentRotation().Quaternion()).Rotator();

            FVector LocalLocationOffset = CurrentLocalCharacterLocation - InitialLocalCharacterLocation;
            FRotator LocalRotationOffset = CurrentLocalCharacterRotation - InitialLocalCharacterRotation;

            // 로컬 변화량을 현재 SpringArm의 로컬 위치에 더하기
            FVector FinalLocalSpringArmLocation = SpringArmComponent->GetRelativeLocation() + LocalLocationOffset;
            FRotator FinalLocalSpringArmRotation = SpringArmComponent->GetRelativeRotation() + LocalRotationOffset;

            SpringArmComponent->SetRelativeLocation(FinalLocalSpringArmLocation);
            SpringArmComponent->SetRelativeRotation(FinalLocalSpringArmRotation);
        }
    }
}