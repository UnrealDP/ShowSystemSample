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

            // 현재 카메라 위치와 회전을 초기 위치로 저장
            FVector InitialCameraPosition = GetCameraLocation();
            FVector InitialLookAtTarget = GetCameraLookAt();
            InitialFOV = PlayerController->PlayerCameraManager->GetFOVAngle();

            // Actor의 현재 월드 위치 및 회전 가져오기
            FVector ActorWorldPosition = OwnerActor->GetActorLocation();
            FRotator ActorWorldRotation = OwnerActor->GetActorRotation();

            // 초기 카메라 위치의 상대 좌표 저장
            InitialCameraRelativePosition = InitialCameraPosition - ActorWorldPosition;
            InitialLookAtRelativeTarget = InitialLookAtTarget - ActorWorldPosition;

            // 시작 위치를 Actor의 상대 좌표로 저장
            InitialCameraRelativePositionForBlend = InitialCameraRelativePosition;
            InitialLookAtRelativeTargetForBlend = InitialLookAtRelativeTarget;
            InitialFOVForBlend = PlayerController->PlayerCameraManager->GetFOVAngle();
            
            SpringArmComponent = ControlledPawn->FindComponentByClass<USpringArmComponent>();
            //SpringArmComponent->Activate(false);

            InitialLocalCharacterLocation = OwnerActor->GetActorTransform().InverseTransformPosition(SpringArmComponent->GetComponentLocation());
            InitialLocalCharacterRotation = OwnerActor->GetActorTransform().InverseTransformRotation(SpringArmComponent->GetComponentRotation().Quaternion()).Rotator();

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
                ApplyCharacterOffsetToSpringArm();
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
    // Actor의 현재 월드 위치 및 회전 가져오기
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();
    FRotator ActorWorldRotation = OwnerActor->GetActorRotation();

    const FCameraPathPoint& ArrivalPoint = ShowCamSequenceKeyPtr->PathPoints[CurrentPointIndex];
    FVector ArrivalPositionWorld = ActorWorldPosition + ArrivalPoint.Position;
    FVector ArrivalLookAtWorld = ActorWorldPosition + ArrivalPoint.LookAtTarget.GetSafeNormal() * 1000.0f;

    // 보간 계산
    float CurrentDuration = ArrivalPoint.Duration;
    float Alpha = FMath::Clamp(CurrentBlendTime / CurrentDuration, 0.0f, 1.0f);

    // Actor의 현재 위치를 기반으로 월드 좌표 계산
    FVector InitialPositionWorld = ActorWorldPosition + InitialCameraRelativePositionForBlend;
    FVector InterpolatedPosition = FMath::Lerp(InitialPositionWorld, ArrivalPositionWorld, Alpha);

    FVector InitialLookAtWorld = ActorWorldPosition + InitialLookAtRelativeTargetForBlend;
    FVector InterpolatedLookAt = FMath::Lerp(InitialLookAtWorld, ArrivalLookAtWorld, Alpha);
    FRotator InterpolatedRotation = (InterpolatedLookAt - InterpolatedPosition).Rotation();

    // FOV 보간 계산
    TOptional<float> InterpolatedFOV;
    if (ArrivalPoint.FieldOfView.IsSet())
    {
        InterpolatedFOV = FMath::Lerp(InitialFOVForBlend, ArrivalPoint.FieldOfView.GetValue(), Alpha);
    }

    // 카메라 설정 적용
    ApplyCameraSettings(InterpolatedPosition, InterpolatedRotation, InterpolatedFOV);

    // 시간 증가 및 다음 포인트로 이동 처리
    if (CurrentBlendTime >= CurrentDuration)
    {
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
                FVector InitialLookAtTargetForBlend = GetCameraLookAt();
                InitialFOVForBlend = PlayerController->PlayerCameraManager->GetFOVAngle();

                InitialCameraRelativePositionForBlend = InitialCameraLocationForBlend - ActorWorldPosition;
                InitialLookAtRelativeTargetForBlend = InitialLookAtTargetForBlend - ActorWorldPosition;
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
}

void UShowCamSequence::PathPointReturningToStart(AActor* OwnerActor)
{
    // Actor의 현재 월드 위치 및 회전 가져오기
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();
    FRotator ActorWorldRotation = OwnerActor->GetActorRotation();

    FVector ArrivalPositionWorld = ActorWorldPosition + InitialCameraRelativePosition;

    // 보간 계산: 상대 좌표 기반
    float Alpha = FMath::Clamp(CurrentBlendTime / ShowCamSequenceKeyPtr->FadeOutBlendTime, 0.0f, 1.0f);

    FVector InitialPositionWorld = ActorWorldPosition + InitialCameraRelativePositionForBlend;
    FVector InterpolatedPosition = FMath::Lerp(InitialPositionWorld, ArrivalPositionWorld, Alpha);

    FVector InitialLookAtWorld = ActorWorldPosition + InitialLookAtRelativeTargetForBlend;
    FVector ArrivalLookAtWorld = ActorWorldPosition + InitialLookAtRelativeTarget;
    FVector InterpolatedLookAt = FMath::Lerp(InitialLookAtWorld, ArrivalLookAtWorld, Alpha);
    FRotator InterpolatedRotation = (InterpolatedLookAt - InterpolatedPosition).Rotation();

    // FOV 보간 계산
    TOptional<float> InterpolatedFOV;
    if (InitialFOV != InitialFOVForBlend)
    {
        InterpolatedFOV = FMath::Lerp(InitialFOVForBlend, InitialFOV, Alpha);
    }

    // 카메라 설정 적용
    ApplyCameraSettings(InterpolatedPosition, InterpolatedRotation, InterpolatedFOV);

    // 복귀 완료 시 상태 종료
    if (CurrentBlendTime >= ShowCamSequenceKeyPtr->FadeOutBlendTime)
    {
        State = ECameraSequenceState::End;
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
        
        if (SpringArmComponent)
        {
            SpringArmComponent->OnRegister();
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