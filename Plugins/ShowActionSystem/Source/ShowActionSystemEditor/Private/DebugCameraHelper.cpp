// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCameraHelper.h"
#include "RunTime/ShowKeys/ShowCamSequence.h"
#include "GizmoTranslationComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/TextRenderComponent.h"

// Sets default values
ADebugCameraHelper::ADebugCameraHelper()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADebugCameraHelper::BeginPlay()
{
	Super::BeginPlay();
}

void ADebugCameraHelper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (FDebugCamera* DebugCamera : DebugCameras)
    {
        delete DebugCamera;
    }
    DebugCameras.Empty();

    CameraPathLineBatch = nullptr;

    Super::EndPlay(EndPlayReason);
}

void ADebugCameraHelper::Initialize(AActor* InShowOwnerActor, TObjectPtr<UShowCamSequence> InShowCamSequence)
{
    checkf(InShowOwnerActor, TEXT("ADebugCameraHelper::Initialize InShowOwnerActor Invalid"));
    checkf(InShowCamSequence, TEXT("ADebugCameraHelper::Initialize InShowCamSequence Invalid"));

    ShowOwnerActor = InShowOwnerActor;
    ShowCamSequence = InShowCamSequence;

    FVector ShowOwnerActorLocation = ShowOwnerActor->GetActorLocation();
    for (int i = 0; i < ShowCamSequence->GetShowCamSequenceKeyPtr()->PathPoints.Num(); ++i)
    {
        FDebugCamera* DebugCamera = CreateMesh(i, ShowCamSequence->GetShowCamSequenceKeyPtr()->PathPoints[i]);
        DebugCameras.Add(DebugCamera);

        UpdateDebugCamera(
            DebugCamera, 
            ShowOwnerActorLocation + ShowCamSequence->GetShowCamSequenceKeyPtr()->PathPoints[i].Position,
            ShowOwnerActorLocation + ShowCamSequence->GetShowCamSequenceKeyPtr()->PathPoints[i].LookAtTarget);
    }

    CameraPathLineBatch = NewObject<ULineBatchComponent>(this, ULineBatchComponent::StaticClass());
    AddInstanceComponent(CameraPathLineBatch);
    CameraPathLineBatch->RegisterComponent();

    DrawCameraPath();
}

FDebugCamera* ADebugCameraHelper::CreateMesh(const int Index, const FCameraPathPoint& CameraPathPoint)
{
    FDebugCamera* DebugCamera = new FDebugCamera();
    DebugCamera->CameraPathPointPtr = const_cast<FCameraPathPoint*>(&CameraPathPoint);

    // 디버그 카메라 기즈모 생성
    DebugCamera->DebugCameraGizmo = NewObject<UGizmoTranslationComponent>(this, UGizmoTranslationComponent::StaticClass());
    DebugCamera->DebugCameraGizmo->Initialize();
    AddInstanceComponent(DebugCamera->DebugCameraGizmo);
    DebugCamera->DebugCameraGizmo->RegisterComponent();
    DebugCamera->DebugCameraGizmo->SetRelativeScale3D(FVector(1.0f));
    DebugCamera->DebugCameraGizmo->OnGizmoAxisUpdated.BindUObject(this, &ADebugCameraHelper::UpdatePath);

    // 디버그 카메라 메쉬 생성
    DebugCamera->DebugCameraMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
    AddInstanceComponent(DebugCamera->DebugCameraMesh);
    DebugCamera->DebugCameraMesh->RegisterComponent();

    TSoftObjectPtr<UStaticMesh> CameraMeshPath = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam")));
    TObjectPtr<UStaticMesh> CameraMesh = CameraMeshPath.Get();
    if (CameraMesh)
    {
        DebugCamera->DebugCameraMesh->SetStaticMesh(CameraMesh);
        DebugCamera->DebugCameraMesh->SetRelativeScale3D(FVector(0.2f));
        DebugCamera->DebugCameraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DebugCamera->DebugCameraMesh->AttachToComponent(DebugCamera->DebugCameraGizmo, FAttachmentTransformRules::KeepRelativeTransform);
    }

    // 디버그 LookAt 기즈모 생성
    DebugCamera->DebugLookAtGizmo = NewObject<UGizmoTranslationComponent>(this, UGizmoTranslationComponent::StaticClass());
    DebugCamera->DebugLookAtGizmo->Initialize();
    AddInstanceComponent(DebugCamera->DebugLookAtGizmo);
    DebugCamera->DebugLookAtGizmo->RegisterComponent();
    DebugCamera->DebugLookAtGizmo->SetRelativeScale3D(FVector(1.0f));
    DebugCamera->DebugLookAtGizmo->OnGizmoAxisUpdated.BindUObject(this, &ADebugCameraHelper::UpdatePath);


    // LookAt 스피어 메쉬 생성
    DebugCamera->LookAtSphereMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
    AddInstanceComponent(DebugCamera->LookAtSphereMesh);
    DebugCamera->LookAtSphereMesh->RegisterComponent();

    TSoftObjectPtr<UStaticMesh> SphereMeshPath = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere.Sphere")));
    TObjectPtr<UStaticMesh> SphereMesh = SphereMeshPath.Get();
    if (SphereMesh)
    {
        DebugCamera->LookAtSphereMesh->SetStaticMesh(SphereMesh);
        DebugCamera->LookAtSphereMesh->SetRelativeScale3D(FVector(0.17f));
        DebugCamera->LookAtSphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DebugCamera->LookAtSphereMesh->AttachToComponent(DebugCamera->DebugLookAtGizmo, FAttachmentTransformRules::KeepRelativeTransform);
    }

    // 카메라 방향 라인 컴포넌트 생성
    DebugCamera->LineBatch = NewObject<ULineBatchComponent>(this, ULineBatchComponent::StaticClass());
    AddInstanceComponent(DebugCamera->LineBatch);
    DebugCamera->LineBatch->RegisterComponent();

    // 카메라 번호 텍스트 컴포넌트 생성
    DebugCamera->TextRenderComponent = NewObject<UTextRenderComponent>(this, UTextRenderComponent::StaticClass());
    AddInstanceComponent(DebugCamera->TextRenderComponent);
    DebugCamera->TextRenderComponent->RegisterComponent();
    DebugCamera->TextRenderComponent->SetText(FText::AsNumber(Index));
    DebugCamera->TextRenderComponent->SetHorizontalAlignment(EHTA_Center);
    DebugCamera->TextRenderComponent->SetWorldSize(25.0f);
    DebugCamera->TextRenderComponent->SetTextRenderColor(FColor::Purple);

    return DebugCamera;
}

void ADebugCameraHelper::DrawCameraPath()
{
    if (!CameraPathLineBatch)
    {
        return;
    }

    CameraPathLineBatch->Flush();

    float PlaybackEndTime = 0.0f;
    FInterpCurve<FVector> PositionCurve;
    UShowCamSequence::CreateCurve(
        PlaybackEndTime,
        ShowCamSequence->GetShowCamSequenceKeyPtr(),
        &PositionCurve,
        nullptr,
        nullptr,
        0.0f);

    PositionCurve.AutoSetTangents(0.0f, ShowCamSequence->GetShowCamSequenceKeyPtr()->bStationaryEndpoints);

    AActor* ShowOwner = ShowCamSequence->GetShowOwner();
    FVector OwnerLocation = ShowOwner->GetActorLocation();

    const float SamplingInterval = 0.1f; // 샘플링 간격
    FVector PreviousPosition = PositionCurve.Eval(0.0f, FVector::ZeroVector);    
    PreviousPosition = PreviousPosition + OwnerLocation;

    for (float Time = SamplingInterval; Time <= PositionCurve.Points.Last().InVal; Time += SamplingInterval)
    {
        FVector CurrentPosition = PositionCurve.Eval(Time, FVector::ZeroVector);
        CurrentPosition = CurrentPosition + OwnerLocation;

        // 이전 위치와 현재 위치를 연결하는 선을 그립니다.
        CameraPathLineBatch->DrawLine(
            PreviousPosition,
            CurrentPosition,
            FLinearColor::Gray,    // 선 색상
            0,                // 선 그룹 ID
            0.5f,             // 선 두께
            -1.0f              // 지속 시간
        );

        PreviousPosition = CurrentPosition;
    }
}

// Called every frame
void ADebugCameraHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    for (FDebugCamera* DebugCamera : DebugCameras)
    {
        if (DebugCamera->CameraPathPointPtr)
        {
            if (DebugCamera->CameraPathPointPtr->bNeedUpdateLocation)
            {
                if (GetCameraLocationDelegate.IsBound())
                {
                    FVector Location = GetCameraLocationDelegate.Execute();
                    AActor* ShowOwner = ShowCamSequence->GetShowOwner();
                    FVector OwnerLocation = ShowOwner->GetActorLocation();

                    DebugCamera->CameraPathPointPtr->Position = Location - OwnerLocation;
                    UpdateDebugCamera(DebugCamera, OwnerLocation + DebugCamera->CameraPathPointPtr->Position, OwnerLocation + DebugCamera->CameraPathPointPtr->LookAtTarget);
                    DebugCamera->CameraPathPointPtr->bNeedUpdateLocation = false;

                    if (OnUpdate.IsBound())
                    {
                        OnUpdate.Execute();
                    }

                    if (OnUpdateCameraPathPoint.IsBound())
                    {
                        OnUpdateCameraPathPoint.Execute(DebugCamera->CameraPathPointPtr);
                    }
                }
            }

            if (DebugCamera->CameraPathPointPtr->bIsSelected)
            {
                if (SelectedCameraPathPoint != DebugCamera->CameraPathPointPtr)
				{
					SelectedCameraPathPoint = DebugCamera->CameraPathPointPtr;
                    if (OnUpdateCameraPathPoint.IsBound())
                    {
                        OnUpdateCameraPathPoint.Execute(DebugCamera->CameraPathPointPtr);
                    }
				}
            }
        }

        if (DebugCamera->TextRenderComponent)
        {
#if WITH_EDITOR
            if (GEditor->bIsSimulatingInEditor)
            {
                FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
                FVector CameraLocation = EditorViewportClient->GetViewLocation();
                FVector TextLocation = DebugCamera->TextRenderComponent->GetComponentLocation();

                // 텍스트가 카메라를 바라보도록 회전 계산
                FRotator LookAtRotation = (CameraLocation - TextLocation).Rotation();
                DebugCamera->TextRenderComponent->SetWorldRotation(LookAtRotation);
            }
            else
#endif    
            {
                APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
                if (PlayerController && PlayerController->PlayerCameraManager)
                {
                    FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
                    FVector TextLocation = DebugCamera->TextRenderComponent->GetComponentLocation();

                    // 텍스트가 카메라를 바라보도록 회전 계산
                    FRotator LookAtRotation = (CameraLocation - TextLocation).Rotation();
                    DebugCamera->TextRenderComponent->SetWorldRotation(LookAtRotation);
                }
            }
        }
    }

    // 기즈모로 움직이는 경우 너무 자주 호출되면 사용성이 떨어질 듯 하여 1초에 한번만 카메라 랜더 타겟 갱신
    if (UpdatedCameraTime > 0.0f)
    {
        UpdatedCameraTime -= DeltaTime;
    }
    if (UpdatedCameraPaths.Num() > 0 && UpdatedCameraTime <= 0.0f)
    {
        for (FCameraPathPoint* CameraPathPoint : UpdatedCameraPaths)
        {
            if (CameraPathPoint == SelectedCameraPathPoint)
			{
				if (OnUpdateCameraPathPoint.IsBound())
				{
					OnUpdateCameraPathPoint.Execute(CameraPathPoint);
				}
                break;
			}
        }
        UpdatedCameraPaths.Empty();
		UpdatedCameraTime = 1.0f;
    }
}

void ADebugCameraHelper::UpdateDebugCamera(FDebugCamera* DebugCamera, const FVector& CameraPos, const FVector& LookAtPos)
{
    checkf(DebugCamera, TEXT("ADebugCameraHelper::UpdateDebugCamera DebugCamera Invalid"));

    int index = DebugCameras.Find(DebugCamera);
    UpdateDebugCamera(index, CameraPos, LookAtPos);
}

void ADebugCameraHelper::UpdateDebugCamera(int index, const FVector& CameraPos, const FVector& LookAtPos)
{
    checkf(index < 0 || index < DebugCameras.Num(), TEXT("ADebugCameraHelper::UpdateDebugCamera Invalid index %d"), index);
    checkf(DebugCameras[index], TEXT("ADebugCameraHelper::UpdateDebugCamera DebugCameras Invalid [ %d ]"), index);

    // LookAt 방향으로 최종 회전 설정
    FRotator LookAtRotation = (LookAtPos - CameraPos).Rotation();
    FQuat LookAtQuat = FQuat(LookAtRotation);
    FQuat OffsetQuat = FQuat(FRotator(0.f, 90.f, 0.f));
    FRotator FinalRotation = (LookAtQuat * OffsetQuat).Rotator();
    DebugCameras[index]->DebugCameraMesh->SetWorldRotation(FinalRotation);
    DebugCameras[index]->DebugCameraGizmo->SetWorldLocation(CameraPos);

    // LookAt 스피어 위치 업데이트
    DebugCameras[index]->DebugLookAtGizmo->SetWorldLocation(LookAtPos);

    // 카메라와 LookAt 사이에 디버그 선 그리기
    DebugCameras[index]->LineBatch->Flush(); // 이전 프레임의 라인 제거
    DebugCameras[index]->LineBatch->DrawLine(
        DebugCameras[index]->DebugCameraMesh->GetComponentLocation(),
        DebugCameras[index]->LookAtSphereMesh->GetComponentLocation(),
        FLinearColor::Red, 0, 0.5f);

    if (SelectedCameraPathPoint != DebugCameras[index]->CameraPathPointPtr)
	{
        for (FDebugCamera* DebugCamera : DebugCameras)
        {
            DebugCamera->CameraPathPointPtr->bIsSelected = false;
        }
        DebugCameras[index]->CameraPathPointPtr->bIsSelected = true;
	}

    DebugCameras[index]->TextRenderComponent->SetWorldLocation(CameraPos);
    DebugCameras[index]->TextRenderComponent->AddRelativeLocation(FVector(0.0f, -15.0f, 0.0f));

    DrawCameraPath();
}

void ADebugCameraHelper::UpdatePath(UGizmoTranslationComponent* GizmoComponent)
{
	checkf(GizmoComponent, TEXT("ADebugCameraHelper::UpdatePath GizmoComponent Invalid"));

    AActor* ShowOwner = ShowCamSequence->GetShowOwner();
    FVector OwnerLocation = ShowOwner->GetActorLocation();

    FShowCamSequenceKey* ShowCamSequenceKey = const_cast<FShowCamSequenceKey*>(ShowCamSequence->GetShowCamSequenceKeyPtr());
    TArray<FCameraPathPoint>* PathPointsPtr = (&ShowCamSequenceKey->PathPoints);

    for (int i = 0; i < PathPointsPtr->Num(); ++i)
    {
        FCameraPathPoint& CameraPathPoint = (*PathPointsPtr)[i];
        FDebugCamera* DebugCamera = DebugCameras[i];

        FVector NewPosition = DebugCamera->DebugCameraMesh->GetComponentLocation() - OwnerLocation;
        FVector NewLookAtTarget = DebugCamera->LookAtSphereMesh->GetComponentLocation() - OwnerLocation;

        if (!CameraPathPoint.Position.Equals(NewPosition) || !CameraPathPoint.LookAtTarget.Equals(NewLookAtTarget))
        {
            CameraPathPoint.Position = NewPosition;
            CameraPathPoint.LookAtTarget = NewLookAtTarget;

            UpdateDebugCamera(DebugCamera, OwnerLocation + CameraPathPoint.Position, OwnerLocation + CameraPathPoint.LookAtTarget);

            if (!UpdatedCameraPaths.Contains(DebugCamera->CameraPathPointPtr))
            {
                UpdatedCameraPaths.Add(DebugCamera->CameraPathPointPtr);
            }
        }
    }

    if (OnUpdate.IsBound())
	{
		OnUpdate.Execute();
	}
}
