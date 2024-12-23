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

    PlaybackEndTime = 0.0f;
    PositionCurve.Reset();
    LookAtCurve.Reset();
    FOVCurve.Reset();

#if WITH_EDITOR
    bIsShowDebugCamera = false;
    if (DebugCameraMesh)
    {
        DebugCameraMesh->RemoveFromRoot();
        DebugCameraMesh->UnregisterComponent();
        DebugCameraMesh->DestroyComponent();
        DebugCameraMesh = nullptr;
    }
#endif
}

void UShowCamSequence::Reset()
{
    ShowCamSequenceKeyPtr = nullptr;
    State = ECameraSequenceState::Wait;
    CurrentBlendTime = 0.0f;
    CurrentPointIndex = 0;

    PlaybackEndTime = 0.0f;
    PositionCurve.Reset();
    LookAtCurve.Reset();
    FOVCurve.Reset();

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
        }
    }

    if (SpringArmComponent && CameraComponent)
    {
        // 카메라의 현재 위치와 LookAt 을 캐릭터 기준으로 계산하고 저장
        FVector ActorWorldPosition = OwnerActor->GetActorLocation();
        PreviousLocation = GetCameraLocation();
        InitialRelativeLocationFromSocket = PreviousLocation - ActorWorldPosition;

        PreviousLookAt = GetCameraLookAt();
        InitialRelativeLookAtFromSocket = PreviousLookAt - ActorWorldPosition;

        InitialFOV = GetCameraFOV();

        bInitialUsePawnControlRotation = SpringArmComponent->bUsePawnControlRotation;

        // SpringArm 비활성화
        SpringArmComponent->Activate(false);
        SpringArmComponent->bUsePawnControlRotation = false;
        SpringArmComponent->SetComponentTickEnabled(false);
    }

    FVector ActorWorldPosition = OwnerActor->GetActorLocation();

    // 최초 시작 위치 설정 (캐릭터 상대 좌표로 저장)
    FVector CrrRelativeLocation = GetCameraLocation() - ActorWorldPosition;
    FVector CrrRelativeLoockAt = GetCameraLookAt() - ActorWorldPosition;
    AddVectorPoint(PositionCurve, 0.0f, CrrRelativeLocation, ShowCamSequenceKeyPtr->FadeInMode);
    LookAtCurve.AddPoint(0.0f, CrrRelativeLoockAt);
    FOVCurve.AddPoint(0.0f, GetCameraFOV());

    // 각 구간별 Curve Add
    UShowCamSequence::CreateCurve(
        PlaybackEndTime,
        ShowCamSequenceKeyPtr,
        &PositionCurve,
        &LookAtCurve,
        &FOVCurve,
        GetCameraFOV());

    // 마지막 종료 구간 Curve 등록
    PlaybackEndTime += ShowCamSequenceKeyPtr->FadeOutBlendTime;
    switch (ShowCamSequenceKeyPtr->CameraReturnOption)
    {
    case ECameraReturnOption::ReturnToStart:
    {
        PositionCurve.AddPoint(PlaybackEndTime, CrrRelativeLocation);
        LookAtCurve.AddPoint(PlaybackEndTime, CrrRelativeLoockAt);
        FOVCurve.AddPoint(PlaybackEndTime, GetCameraFOV());
        break;
    }
    case ECameraReturnOption::ReturnToDefault:
    {
        APawn* ControlledPawn = PlayerController->GetPawn();
        if (ControlledPawn && SpringArmComponent)
        {
            FVector CurrentForwardVector = ControlledPawn->GetActorForwardVector();

            FRotator CurrentRotation = ControlledPawn->GetActorRotation();
            FVector InitialForwardVector = CurrentRotation.UnrotateVector(CurrentForwardVector);

            FVector DefaultCameraLocation = -InitialForwardVector * SpringArmComponent->TargetArmLength
                + SpringArmComponent->SocketOffset
                + SpringArmComponent->TargetOffset;

            PositionCurve.AddPoint(PlaybackEndTime, DefaultCameraLocation);
        }
        else
        {
            PositionCurve.AddPoint(PlaybackEndTime, CrrRelativeLocation);
        }
        
        LookAtCurve.AddPoint(PlaybackEndTime, CrrRelativeLoockAt);
        FOVCurve.AddPoint(PlaybackEndTime, GetCameraFOV());
        break;
    }
    case ECameraReturnOption::MaintainEndPosition:
    {
        PositionCurve.AddPoint(PlaybackEndTime, ShowCamSequenceKeyPtr->PathPoints.Last().Position);
        LookAtCurve.AddPoint(PlaybackEndTime, CrrRelativeLoockAt);
        FOVCurve.AddPoint(PlaybackEndTime, GetCameraFOV());
        break;
    }
    default:
        break;
    }
    
    PositionCurve.AutoSetTangents(0.0f, ShowCamSequenceKeyPtr->bStationaryEndpoints);
    LookAtCurve.AutoSetTangents(0.0f, false);
    FOVCurve.AutoSetTangents(0.0f, false);

    State = ECameraSequenceState::Playing;
    CurrentBlendTime = 0.0f;
    CurrentPointIndex = 0;
}

void UShowCamSequence::CreateCurve(
    float& OutPlaybackEndTime, 
    const FShowCamSequenceKey* InShowCamSequenceKeyPtr, 
    FInterpCurve<FVector>* OutPositionCurvePtr,
    FInterpCurve<FVector>* OutLookAtCurvePtr,
    FInterpCurve<float>* OutFOVCurvePtr,
    float FOV)
{
    // 각 구간별 Curve Add
    OutPlaybackEndTime = 0.0f;
    int CurveIdx = 0;
    int CurveNum = InShowCamSequenceKeyPtr->PathPoints.Num();
    for (; CurveIdx < CurveNum; ++CurveIdx)
    {
        OutPlaybackEndTime += InShowCamSequenceKeyPtr->PathPoints[CurveIdx].Duration;

        if (OutPositionCurvePtr)
        {
            AddVectorPoint(*OutPositionCurvePtr, OutPlaybackEndTime, InShowCamSequenceKeyPtr->PathPoints[CurveIdx].Position, InShowCamSequenceKeyPtr->PathPoints[CurveIdx].InterpMode);
        }
        
        if (OutLookAtCurvePtr)
        {
            OutLookAtCurvePtr->AddPoint(OutPlaybackEndTime, InShowCamSequenceKeyPtr->PathPoints[CurveIdx].LookAtTarget);
        }

        if (OutFOVCurvePtr)
        {
            if (InShowCamSequenceKeyPtr->PathPoints[CurveIdx].FieldOfView.IsSet())
            {
                FOV = InShowCamSequenceKeyPtr->PathPoints[CurveIdx].FieldOfView.GetValue();
            }
            OutFOVCurvePtr->AddPoint(OutPlaybackEndTime, FOV);
        }
    }
}

void UShowCamSequence::AddVectorPoint(FInterpCurve<FVector>& Curve, const float InVal, const FVector& OutVal, const ECameraCurveMode Mode)
{
    int Idx = Curve.AddPoint(InVal, OutVal);
    EInterpCurveMode InterpMode = static_cast<EInterpCurveMode>(Mode);
    Curve.Points[Idx].InterpMode = InterpMode;
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
        PathPointPlay(OwnerActor, BasePassedTime);
        break;
    }
    /*case ECameraSequenceState::ReturningToStart:
    {
        PathPointReturningToStart(OwnerActor, ScaleDeltaTime);
        break;
    }*/
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

void UShowCamSequence::PathPointPlay(AActor* OwnerActor, float BasePassedTime)
{
    // Actor의 현재 월드 위치 가져오기0
    FVector ActorWorldPosition = OwnerActor->GetActorLocation();
    FRotator ActorWorldRotation = OwnerActor->GetActorRotation();

    if (BasePassedTime >= PlaybackEndTime)
    {
        FVector Position = PositionCurve.Points.Last().OutVal;
        FVector LookAt = LookAtCurve.Points.Last().OutVal;
        TOptional<float> FOV = FOVCurve.Points.Last().OutVal;

        FVector ArrivalPositionWorld = CalculateRelativePositionForPlayback(Position, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);
        FVector ArrivalLookAtWorld = CalculateRelativePositionForPlayback(LookAt, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);

        TOptional<float> InterpolatedFOV = InitialFOV;
        ApplyCameraSettings(ArrivalPositionWorld, ArrivalLookAtWorld, InterpolatedFOV);

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
        FVector Position = PositionCurve.Eval(BasePassedTime, PositionCurve.Points.Last().OutVal);
        FVector LookAt = LookAtCurve.Eval(BasePassedTime, LookAtCurve.Points.Last().OutVal);
        TOptional<float> FOV = FOVCurve.Eval(BasePassedTime, FOVCurve.Points.Last().OutVal);

        FVector ArrivalPositionWorld = CalculateRelativePositionForPlayback(Position, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);
        FVector ArrivalLookAtWorld = CalculateRelativePositionForPlayback(LookAt, ActorWorldPosition, ActorWorldRotation, ShowCamSequenceKeyPtr->CameraSequenceOption);
        ApplyCameraSettings(ArrivalPositionWorld, ArrivalLookAtWorld, FOV);
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
        ApplyCameraSettings(ArrivalPositionWorld, ArrivalLookAtWorld, InterpolatedFOV);

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
        float CamaraFov = GetCameraFOV();
        if (InitialFOV != CamaraFov)
        {
            InterpolatedFOV = FMath::Lerp(CamaraFov, InitialFOV, Alpha);
        }
        ApplyCameraSettings(InterpolatedPosition, InterpolatedLookAt, InterpolatedFOV);
    }
}

void UShowCamSequence::ApplyCameraSettings(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV)
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

    if (bIsShowDebugCamera && DebugCameraMesh)
    {
        DebugCameraMesh->SetWorldLocation(PreviousLocation);

        FQuat LookAtQuat = FQuat(Rotation);
        FQuat OffsetQuat = FQuat(FRotator(0.f, 90.f, 0.f));
        FRotator FinalRotation = (LookAtQuat * OffsetQuat).Rotator();
        DebugCameraMesh->SetWorldRotation(FinalRotation);
    }
    else
    {
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
}

void UShowCamSequence::ShowSwitchDebugCameara(bool On)
{
    if (bIsShowDebugCamera == On)
    {
		return;
	}

    bIsShowDebugCamera = On;
    
    if (bIsShowDebugCamera)
    {
        if (DebugCameraMesh)
        {
            DebugCameraMesh->SetVisibility(true);
        }
        else
        {
            // 디버그 카메라 메쉬 생성
            AActor* Owner = GetShowOwner();
            DebugCameraMesh = NewObject<UStaticMeshComponent>(Owner, UStaticMeshComponent::StaticClass());
            DebugCameraMesh->AddToRoot();
            Owner->AddInstanceComponent(DebugCameraMesh);
            DebugCameraMesh->RegisterComponent();

            TSoftObjectPtr<UStaticMesh> CameraMeshPath = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam")));
            TObjectPtr<UStaticMesh> CameraMesh = CameraMeshPath.Get();
            if (CameraMesh)
            {
                DebugCameraMesh->SetStaticMesh(CameraMesh);
                DebugCameraMesh->SetRelativeScale3D(FVector(0.2f));
                DebugCameraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
    else
    {
        if (DebugCameraMesh)
        {
            DebugCameraMesh->SetVisibility(false);
        }
    }
    
}
#endif

FVector UShowCamSequence::GetCameraLocation()
{
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
        return EditorViewportClient->GetViewLocation();
    }
#endif    

    if (!CameraComponent)
    {
        return FVector::ZeroVector;
    }

    return CameraComponent->GetComponentLocation();
}

FVector UShowCamSequence::GetCameraLookAt()
{
    FVector CameraLocation;
    FVector CameraDirection;
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
        FVector LookAtTarget = EditorViewportClient->GetLookAtLocation();
        return LookAtTarget;
    }
#endif    
    else
    {
        if (!CameraComponent || !SpringArmComponent)
        {
            return FVector::ZeroVector;
        }

        CameraLocation = CameraComponent->GetComponentLocation();
        CameraDirection = CameraComponent->GetForwardVector().GetSafeNormal();
        FVector LookAtTarget = CameraLocation + (CameraDirection * SpringArmComponent->TargetArmLength);
        return LookAtTarget;
    }
}

float UShowCamSequence::GetCameraFOV()
{
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());        
        return EditorViewportClient->ViewFOV;
    }
#endif    
    else
    {
        if (!PlayerController || !PlayerController->PlayerCameraManager)
        {
            return 0.0f;
        }

        return PlayerController->PlayerCameraManager->GetFOVAngle();
    }
}

void UShowCamSequence::SetPassedTime(float InTime)
{
    if (State != ECameraSequenceState::Playing)
	{
        Play();
	}

    AActor* OwnerActor = GetShowOwner();
    if (OwnerActor)
    {
        PathPointPlay(OwnerActor, InTime);
    }
}
