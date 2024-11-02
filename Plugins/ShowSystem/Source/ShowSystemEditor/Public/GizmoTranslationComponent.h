// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Math/Axis.h"
#include "GizmoTranslationComponent.generated.h"

class UArrowComponent;
struct FCameraPathPoint;

// 기즈모 위치 업데이트 델리게이트 선언
DECLARE_DELEGATE_OneParam(FOnGizmoAxisUpdated, UGizmoTranslationComponent*);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOWSYSTEMEDITOR_API UGizmoTranslationComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// 현재 활성화된 기즈모 인스턴스
	static UGizmoTranslationComponent* ActiveGizmo;

	// Sets default values for this component's properties
	UGizmoTranslationComponent();
	void Initialize();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	bool IsSimulating() const;
	void HandleSimulationMode();
	bool DetectArrowHit(FVector MouseLocation, FVector MouseDirection, FHitResult& OutHit);
	void HandleRuntimeMode();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void OnMouseOver(UArrowComponent* HoveredArrow);
	void OnMouseClick(FVector HitLocation, UArrowComponent* HitArrow, FIntPoint MousePos);
	void OnMouseRelease();

	FOnGizmoAxisUpdated OnGizmoAxisUpdated;

private:

	UArrowComponent* CreateArrow(FName SubobjectFName);
	void StartDragging(FVector HitLocation, UArrowComponent* HitArrow, FIntPoint MousePos);
	void StopDragging();
	void UpdateArrowColor(UArrowComponent* ArrowComponent, bool bIsSelected);

	// 선택된 축 반환 함수
	EAxis::Type GetAxisFromArrow(UArrowComponent* ArrowComponent) const;


private:

	// 축별 화살표 메쉬
	UPROPERTY()
	UArrowComponent* XAxisArrow = nullptr;

	UPROPERTY()
	UArrowComponent* YAxisArrow = nullptr;

	UPROPERTY()
	UArrowComponent* ZAxisArrow = nullptr;

	FCameraPathPoint* CameraPathPoint;

	bool bIsDragging = false;
	FIntPoint BeforeMousePos = FIntPoint::ZeroValue;
	UArrowComponent* SelectedArrow = nullptr;

	// 축별 기본 색상과 선택된 색상
	FColor DefaultXColor = FColor::Red;
	FColor DefaultYColor = FColor::Green;
	FColor DefaultZColor = FColor::Blue;
	FColor SelectedColor = FColor::Yellow;
};
