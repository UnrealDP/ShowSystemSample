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

    Length += ShowCamSequenceKeyPtr->FadeOutBlendTime;
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
                // 카메라의 현재 위치와 LookAt 을 캐릭터 기준으로 계산하고 저장
                FVector ActorWorldPosition = OwnerActor->GetActorLocation();
                PreviousLocation = GetCameraLocation();
                InitialRelativeLocationFromSocket = PreviousLocation - ActorWorldPosition;

                PreviousLookAt = GetCameraLookAt();
                InitialRelativeLookAtFromSocket = PreviousLookAt - ActorWorldPosition;

                InitialFOV = PlayerController->PlayerCameraManager->GetFOVAngle();

                bInitialUsePawnControlRotation = SpringArmComponent->bUsePawnControlRotation;

                // SpringArm 비활성화
                SpringArmComponent->Activate(false);
                SpringArmComponent->bUsePawnControlRotation = false;
                SpringArmComponent->SetComponentTickEnabled(false);
            }
        }
    }

    State = ECameraSequenceState::Playing;
    CurrentBlendTime = 0.0f;
    CurrentPointIndex = 0;
}

FVector UShowCamSequence::CalculateRelativePositionForPlayback(const FVector& OriginVactor, const FVector& ActorWorldPosition, const FRotator& ActorWorldRotation, ECameraSequenceOption Option) const
{
    switch (Option)
    {
    case ECameraSequenceOption::PlayerForward:
        return ActorWorldRotation.RotateVector(OriginVactor) + ActorWorldPosition;

    case ECameraSequenceOption::Player:
        return OriginVactor + ActorWorldPosition;

    case ECameraSequenceOption::World:
        return OriginVactor;

    default:
        return FVector::ZeroVector;
    }
}

void UShowCamSequence::Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime)
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

    CurrentBlendTime += ScaleDeltaTime;

    switch (State)
    {
    case ECameraSequenceState::Playing:
    {
        PathPointPlay(OwnerActor, ScaleDeltaTime);
        break;
    }
    case ECameraSequenceState::ReturningToStart:
    {
        PathPointReturningToStart(OwnerActor, ScaleDeltaTime);
        break;
    }
    case ECameraSequenceState::End:
        if (SpringArmComponent)
        {
            SpringArmComponent->bUsePawnControlRotation = bInitialUsePawnControlRotation;
            SpringArmComponent->Activate(true);
            SpringArmComponent->SetComponentTickEnabled(true);
        }
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;

    default:
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }
}

void UShowCamSequence::PathPointPlay(AActor* OwnerActor, float DeltaTime)
{
    const FCameraPathPoint& ArrivalPoint = ShowCamSequenceKeyPtr->PathPoints[CurrentPointIndex];

    // Actor의 현재 월드 위치 가져오기
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();
    FRotator ActorWorldRotation = OwnerActor->GetActorRotation();

    // 연출 남은시간
    float RemainDuration = ArrivalPoint.Duration - CurrentBlendTime;

    // 목표 위치
    FVector ArrivalPositionWorld = CalculateRelativePositionForPlayback(ArrivalPoint.Position, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);

    // 목표 LookAt
    FVector ArrivalLookAtWorld = ArrivalPoint.LookAtTarget;
    ArrivalLookAtWorld = CalculateRelativePositionForPlayback(ArrivalLookAtWorld, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);

    // 시간 증가 및 다음 포인트로 이동 처리
    if (RemainDuration <= 0)
    {
        TOptional<float> InterpolatedFOV;
        if (ArrivalPoint.FieldOfView.IsSet())
        {
            InterpolatedFOV = ArrivalPoint.FieldOfView.GetValue();
        }

        ApplyCameraSettings(ArrivalPositionWorld, ArrivalLookAtWorld, InterpolatedFOV, DeltaTime);


        CurrentBlendTime = 0.0f;
        CurrentPointIndex++;
        if (CurrentPointIndex >= ShowCamSequenceKeyPtr->PathPoints.Num())
        {
            State = ECameraSequenceState::ReturningToStart;
        }
    }
    else
    {
        // 보간 계산
        float Alpha = FMath::Clamp(DeltaTime / RemainDuration, 0.0f, 1.0f);

        FVector InterpolatedPosition = FMath::Lerp(PreviousLocation, ArrivalPositionWorld, Alpha);
        FVector InterpolatedLookAt = FMath::Lerp(PreviousLookAt, ArrivalLookAtWorld, Alpha);

        // FOV 보간 계산
        TOptional<float> InterpolatedFOV;
        if (ArrivalPoint.FieldOfView.IsSet())
        {
            InterpolatedFOV = FMath::Lerp(PlayerController->PlayerCameraManager->GetFOVAngle(), ArrivalPoint.FieldOfView.GetValue(), Alpha);
        }

        // 카메라 설정 적용
        ApplyCameraSettings(InterpolatedPosition, InterpolatedLookAt, InterpolatedFOV, DeltaTime);
    }
}

void UShowCamSequence::PathPointReturningToStart(AActor* OwnerActor, float DeltaTime)
{
    if (!SpringArmComponent)
    {
        State = ECameraSequenceState::End;
        UE_LOG(LogTemp, Error, TEXT("UShowCamSequence::PathPointReturningToStart SpringArmComponent is null."));
        return;
    }

    // 보간 계산
    float RemainDuration = ShowCamSequenceKeyPtr->FadeOutBlendTime - CurrentBlendTime;
    float Alpha = FMath::Clamp(DeltaTime / RemainDuration, 0.0f, 1.0f);

    FVector InterpolatedPosition;
    FVector ArrivalPositionWorld;
    FVector ArrivalLookAtWorld;
    FVector InterpolatedLookAt;
    switch (ShowCamSequenceKeyPtr->CameraReturnOption)
    {
        case ECameraReturnOption::ReturnToStart:
        {
            // Actor의 현재 위치를 기반으로 월드 좌표 계산
            FVector ActorWorldPosition = OwnerActor->GetActorLocation();
            ArrivalPositionWorld = InitialRelativeLocationFromSocket + ActorWorldPosition;
            InterpolatedPosition = FMath::Lerp(PreviousLocation, ArrivalPositionWorld, Alpha);

            ArrivalLookAtWorld = InitialRelativeLookAtFromSocket + ActorWorldPosition;
            InterpolatedLookAt = FMath::Lerp(PreviousLookAt, ArrivalLookAtWorld, Alpha);
            break;
        }
        case ECameraReturnOption::MaintainEndPosition:
        {
            ArrivalPositionWorld = CameraComponent->GetComponentLocation();
            ArrivalPositionWorld = GetCameraLocation();
            InterpolatedPosition = FMath::Lerp(PreviousLocation, ArrivalPositionWorld, Alpha);

            ArrivalLookAtWorld = GetCameraLookAt();
            InterpolatedLookAt = FMath::Lerp(PreviousLookAt, ArrivalLookAtWorld, Alpha);
            break;
        }
        default:
            break;
    }

    // 복귀 완료 시 상태 종료
    if (CurrentBlendTime >= ShowCamSequenceKeyPtr->FadeOutBlendTime)
    {
        TOptional<float> InterpolatedFOV = InitialFOV;
        ApplyCameraSettings(ArrivalPositionWorld, ArrivalLookAtWorld, InterpolatedFOV, DeltaTime);

        switch (ShowCamSequenceKeyPtr->CameraReturnOption)
        {
            case ECameraReturnOption::ReturnToStart:
            {
                CameraComponent->SetRelativeLocation(FVector::ZeroVector);
                CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
                break;
            }
            case ECameraReturnOption::MaintainEndPosition:
            {
                if (CameraComponent)
                {
                    float NewTargetArmLength = FVector::Dist(ArrivalPositionWorld, OwnerActor->GetActorLocation());
                    SpringArmComponent->TargetArmLength = NewTargetArmLength;

                    FVector Direction = (OwnerActor->GetActorLocation() - ArrivalPositionWorld).GetSafeNormal();
                    FRotator TargetRotation = Direction.Rotation();

                    // PlayerController의 회전을 TargetRotation으로 설정
                    if (PlayerController)
                    {
                        PlayerController->SetControlRotation(TargetRotation);
                    }

                    CameraComponent->SetRelativeLocation(FVector::ZeroVector);
                    CameraComponent->SetRelativeRotation(FRotator::ZeroRotator);
                    SpringArmComponent->bUsePawnControlRotation = bInitialUsePawnControlRotation;

                    SpringArmComponent->TickComponent(0.0f, ELevelTick::LEVELTICK_TimeOnly, nullptr);
                    SpringArmComponent->bUsePawnControlRotation = false;
                }
                break;
            }
            default:
                break;
        }

        State = ECameraSequenceState::End;
    }
    else
    {
        // FOV 보간 계산
        TOptional<float> InterpolatedFOV;
        float CamaraFov = PlayerController->PlayerCameraManager->GetFOVAngle();
        if (InitialFOV != CamaraFov)
        {
            InterpolatedFOV = FMath::Lerp(CamaraFov, InitialFOV, Alpha);
        }
        ApplyCameraSettings(InterpolatedPosition, InterpolatedLookAt, InterpolatedFOV, DeltaTime);
    }
}

void UShowCamSequence::ApplyCameraSettings(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV, float DeltaTime)
{
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        ApplyEditorViewCamera(NewPosition, NewLookAt, NewFOV);
    }
    else
#endif
    {
        ApplyRunTimeCamera(NewPosition, NewLookAt, NewFOV);
    }
}

void UShowCamSequence::ApplyRunTimeCamera(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV)
{
    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }

    // FOV 설정
    if (NewFOV.IsSet())
    {
        PlayerController->PlayerCameraManager->SetFOV(NewFOV.GetValue());
    }

    AActor* OwnerActor = GetShowOwner();
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ShowCamSequence), false, OwnerActor);

    PreviousLocation = NewPosition;
    PreviousLookAt = NewLookAt;

    UWorld* World = GetWorld();
    if (World)
    {
        FHitResult Result;
        World->SweepSingleByChannel(
            Result,
            NewLookAt,
            NewPosition,
            FQuat::Identity,
            SpringArmComponent->ProbeChannel,
            FCollisionShape::MakeSphere(SpringArmComponent->ProbeSize),
            QueryParams);
        if (Result.bBlockingHit)
        {
            PreviousLocation = Result.Location;
            // 시작 위치인 LookAt과 충돌 후 위치가 같으면 시작 지점과 종료 지점 모두 충돌 영역인거
            if (NewLookAt.Equals(Result.Location))
            {
                // 바라볼 지점도 충돌 영역 안이기 떄문에 캐릭터에서 바라볼 지점으로 총돌 여역을 찾아서 위치 조정
                World->SweepSingleByChannel(
                    Result,
                    OwnerActor->GetActorLocation(),
                    NewLookAt,
                    FQuat::Identity,
                    SpringArmComponent->ProbeChannel,
                    FCollisionShape::MakeSphere(SpringArmComponent->ProbeSize),
                    QueryParams);
                if (Result.bBlockingHit)
                {
                    PreviousLocation = Result.Location;
                }
            }
        }
    }

    FRotator Rotation = (PreviousLookAt - PreviousLocation).Rotation();

    if (CameraComponent)
    {
        CameraComponent->SetWorldLocation(PreviousLocation);
        CameraComponent->SetWorldRotation(Rotation);
    }
}

#if WITH_EDITOR
void UShowCamSequence::ApplyEditorViewCamera(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV)
{
    PreviousLocation = NewPosition;
    PreviousLookAt = NewLookAt;

    FRotator Rotation = (PreviousLookAt - PreviousLocation).Rotation();

    if (GEditor && GEditor->GetActiveViewport())
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());

        if (EditorViewportClient)
        {
            if (NewFOV.IsSet())
			{
				EditorViewportClient->ViewFOV = NewFOV.GetValue();
			}

            EditorViewportClient->SetViewLocation(PreviousLocation);
            EditorViewportClient->SetViewRotation(Rotation);

            // 즉시 업데이트
            EditorViewportClient->Invalidate();
        }
    }
}
#endif

FVector UShowCamSequence::GetCameraLocation()
{
    if (!CameraComponent)
    {
        return FVector::ZeroVector;
    }

#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
        return EditorViewportClient->GetViewLocation();
    }
#endif    

    return CameraComponent->GetComponentLocation();
}

FVector UShowCamSequence::GetCameraLookAt()
{
    if (!CameraComponent || !SpringArmComponent)
    {
        return FVector::ZeroVector;
    }

    FVector CameraLocation;
    FVector CameraDirection;
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
        CameraLocation = EditorViewportClient->GetViewLocation();
        CameraDirection = EditorViewportClient->GetViewRotation().Vector();
    }
#endif    
    else
    {
        CameraLocation = CameraComponent->GetComponentLocation();
        CameraDirection = CameraComponent->GetForwardVector().GetSafeNormal();
    }
    
    FVector LookAtTarget = CameraLocation + (CameraDirection * SpringArmComponent->TargetArmLength);
    return LookAtTarget;
}
