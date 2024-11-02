// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugCameraHelper.h"
#include "RunTime/ShowKeys/ShowCamSequence.h"
#include "GizmoTranslationComponent.h"

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

    Super::EndPlay(EndPlayReason);
}

void ADebugCameraHelper::Initialize(AActor* InShowOwnerActor, TObjectPtr<UShowCamSequence> InShowCamSequence)
{
    checkf(InShowOwnerActor, TEXT("ADebugCameraHelper::Initialize InShowOwnerActor Invalid"));
    checkf(InShowCamSequence, TEXT("ADebugCameraHelper::Initialize InShowCamSequence Invalid"));

    ShowOwnerActor = InShowOwnerActor;
    ShowCamSequence = InShowCamSequence;

    FVector ShowOwnerActorLocation = ShowOwnerActor->GetActorLocation();
    UE_LOG(LogTemp, Warning, TEXT("ShowOwnerActorLocation : %s"), *ShowOwnerActorLocation.ToString());
    for (const FCameraPathPoint& CameraPathPoint : ShowCamSequence->GetShowCamSequenceKeyPtr()->PathPoints)
    {
        FDebugCamera* DebugCamera = CreateMesh(CameraPathPoint);
        DebugCameras.Add(DebugCamera);
        UpdateDebugCamera(DebugCamera, ShowOwnerActorLocation + CameraPathPoint.Position, ShowOwnerActorLocation + CameraPathPoint.LookAtTarget);
    }
}

FDebugCamera* ADebugCameraHelper::CreateMesh(const FCameraPathPoint& CameraPathPoint)
{
    FDebugCamera* DebugCamera = new FDebugCamera();

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
        DebugCamera->DebugCameraMesh->SetRelativeScale3D(FVector(0.3f));
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

    return DebugCamera;
}

// Called every frame
void ADebugCameraHelper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


    for (const FDebugCamera* DebugCamera : DebugCameras)
    {
        // 카메라와 LookAt 사이에 디버그 선 그리기
        DrawDebugLine(
            GetWorld(), 
            DebugCamera->DebugCameraMesh->GetComponentLocation(), 
            DebugCamera->LookAtSphereMesh->GetComponentLocation(),
            FColor::Red, false, -1, 0, 1.0f);
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
}

void ADebugCameraHelper::UpdatePath(UGizmoTranslationComponent* GizmoComponent)
{
	checkf(GizmoComponent, TEXT("ADebugCameraHelper::UpdatePath GizmoComponent Invalid"));

    AActor* ShowOwner = ShowCamSequence->GetShowOwner();
    FVector OwnerLocation = ShowOwner->GetActorLocation();

    FShowCamSequenceKey* ShowCamSequenceKey = const_cast<FShowCamSequenceKey*>(ShowCamSequence->GetShowCamSequenceKeyPtr());
    TArray<FCameraPathPoint>* PathPointsPtr = const_cast<TArray<FCameraPathPoint>*>(&ShowCamSequenceKey->PathPoints);

    for (int i = 0; i < PathPointsPtr->Num(); ++i)
    {
        FCameraPathPoint& CameraPathPoint = (*PathPointsPtr)[i];
        FDebugCamera* DebugCamera = DebugCameras[i];

        CameraPathPoint.Position = DebugCamera->DebugCameraMesh->GetComponentLocation() - OwnerLocation;
        CameraPathPoint.LookAtTarget = DebugCamera->LookAtSphereMesh->GetComponentLocation() - OwnerLocation;

        UpdateDebugCamera(DebugCamera, OwnerLocation + CameraPathPoint.Position, OwnerLocation + CameraPathPoint.LookAtTarget);
    }

    if (OnUpdate.IsBound())
	{
		OnUpdate.Execute();
	}
}
