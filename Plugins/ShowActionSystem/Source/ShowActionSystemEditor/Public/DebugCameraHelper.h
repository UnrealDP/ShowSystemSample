// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugCameraHelper.generated.h"

class UShowCamSequence;
struct FCameraPathPoint;
class UGizmoTranslationComponent;
class ULineBatchComponent;
class UTextRenderComponent;

DECLARE_DELEGATE(FOnUpdate);
DECLARE_DELEGATE_RetVal(FVector, FGetCameraLocationDelegate);
DECLARE_DELEGATE_OneParam(FOnUpdateCameraPathPoint, FCameraPathPoint*);

USTRUCT(BlueprintType)
struct FDebugCamera
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DebugCameraMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGizmoTranslationComponent> DebugCameraGizmo = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LookAtSphereMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGizmoTranslationComponent> DebugLookAtGizmo = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ULineBatchComponent> LineBatch = nullptr;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	FCameraPathPoint* CameraPathPointPtr;
};

UCLASS()
class SHOWACTIONSYSTEMEDITOR_API ADebugCameraHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADebugCameraHelper();
	void Initialize(AActor* InShowOwnerActor, TObjectPtr<UShowCamSequence> InShowCamSequence);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FDebugCamera* CreateMesh(const int Index, const FCameraPathPoint& CameraPathPoint);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void UpdateDebugCamera(FDebugCamera* DebugCamera, const FVector& CameraPos, const FVector& LookAtPos);
	void UpdateDebugCamera(int index, const FVector& CameraPos, const FVector& LookAtPos);
	void UpdatePath(UGizmoTranslationComponent* GizmoComponent);

private:
	AActor* ShowOwnerActor = nullptr;
	TArray<FDebugCamera*> DebugCameras;
	TWeakObjectPtr<UShowCamSequence> ShowCamSequence = nullptr;
	FCameraPathPoint* SelectedCameraPathPoint = nullptr;
	TArray<FCameraPathPoint*> UpdatedCameraPaths;
	float UpdatedCameraTime = 1.0f;

public:	
	FOnUpdate OnUpdate = nullptr;
	FGetCameraLocationDelegate GetCameraLocationDelegate = nullptr;
	FOnUpdateCameraPathPoint OnUpdateCameraPathPoint = nullptr;
};
