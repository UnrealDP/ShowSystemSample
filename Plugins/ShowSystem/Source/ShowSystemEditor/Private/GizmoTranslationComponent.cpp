// Fill out your copyright notice in the Description page of Project Settings.


#include "GizmoTranslationComponent.h"
#include "Components/ArrowComponent.h"

UGizmoTranslationComponent* UGizmoTranslationComponent::ActiveGizmo = nullptr;

// Sets default values for this component's properties
UGizmoTranslationComponent::UGizmoTranslationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UGizmoTranslationComponent::Initialize()
{
    XAxisArrow = CreateArrow(TEXT("XAxisArrow"));
    YAxisArrow = CreateArrow(TEXT("YAxisArrow"));
    ZAxisArrow = CreateArrow(TEXT("ZAxisArrow"));

    // 화살표 기본 색상 설정
    XAxisArrow->SetArrowColor(DefaultXColor);
    YAxisArrow->SetArrowColor(DefaultYColor);
    ZAxisArrow->SetArrowColor(DefaultZColor);

    // 각 축의 방향 설정
    XAxisArrow->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    YAxisArrow->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
    ZAxisArrow->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));

    // 렌더 상태 업데이트
    XAxisArrow->MarkRenderStateDirty();
    YAxisArrow->MarkRenderStateDirty();
    ZAxisArrow->MarkRenderStateDirty();
}

bool UGizmoTranslationComponent::IsSimulating() const
{
#if WITH_EDITOR
    if (GEditor)
    {
        return GEditor->bIsSimulatingInEditor;
    }
    else
#endif  
    {
        return false;
    }
}

UArrowComponent* UGizmoTranslationComponent::CreateArrow(FName SubobjectFName)
{
    UArrowComponent* AxisArrow = NewObject<UArrowComponent>(this, UArrowComponent::StaticClass());
    AxisArrow->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    // 충돌 설정: QueryOnly와 GizmoTrace 채널로 설정
    AxisArrow->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AxisArrow->SetCollisionObjectType(ECC_GameTraceChannel1);

    // 축의 크기
    AxisArrow->ArrowSize = 0.5f;

    // 축의 화살표 길이
    AxisArrow->SetArrowLength(60.0f);

    // 런타임에 숨겨지지 않게
    AxisArrow->bHiddenInGame = false;
    AxisArrow->SetVisibility(true);

    AxisArrow->RegisterComponent();
    return AxisArrow;
}

// Called when the game starts
void UGizmoTranslationComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UGizmoTranslationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ActiveGizmo != nullptr && ActiveGizmo != this)
    {
        return; // 이미 다른 기즈모가 활성화되어 있음
    }

    if (IsSimulating())
    {
        HandleSimulationMode();
    }
    else
    {
        HandleRuntimeMode();
    }
}

void UGizmoTranslationComponent::HandleSimulationMode()
{
    FVector MouseLocation, MouseDirection;

    // 시뮬레이션 모드에서는 EditorViewportClient를 통해 마우스 위치와 입력 상태를 확인
    if (GEditor && GEditor->GetActiveViewport())
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());

        if (EditorViewportClient)
        {
            FViewport* Viewport = EditorViewportClient->Viewport;
            FIntPoint MousePos;
            Viewport->GetMousePos(MousePos);

            FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(Viewport, GetWorld()->Scene, EditorViewportClient->EngineShowFlags));
            FSceneView* View = EditorViewportClient->CalcSceneView(&ViewFamily);

            // 마우스 위치와 방향을 월드 좌표로 변환
            View->DeprojectFVector2D(MousePos, MouseLocation, MouseDirection);

            UArrowComponent* HitArrow = nullptr;
            FHitResult HitResult;
            if (DetectArrowHit(MouseLocation, MouseDirection, HitResult))
            {
                HitArrow = Cast<UArrowComponent>(HitResult.GetComponent());
                if (HitArrow)
                {
                    if (Viewport->KeyState(EKeys::LeftMouseButton) && !bIsDragging)
                    {
                        ActiveGizmo = this;
                        EditorViewportClient->bDisableInput = true;
                        OnMouseClick(HitResult.Location, HitArrow, MousePos);
                    }
                }
            }
            OnMouseOver(HitArrow);

            if (bIsDragging && SelectedArrow)
            {
                // 마우스 이동량 계산
                FIntPoint PosDelta = MousePos - BeforeMousePos;
                EAxis::Type Axis = GetAxisFromArrow(SelectedArrow);

                // 이동 방향 결정
                FVector AxisDirection;
                if (Axis == EAxis::X) AxisDirection = FVector::ForwardVector;
                else if (Axis == EAxis::Y) AxisDirection = FVector::RightVector;
                else if (Axis == EAxis::Z) AxisDirection = FVector::UpVector;

                // 화면에서의 마우스 이동량을 월드 공간으로 변환
                FVector DeltaWorld = View->ViewMatrices.GetViewMatrix().InverseTransformVector(FVector(PosDelta.X, -PosDelta.Y, 0.0f));

                // 선택된 축 방향으로 이동량 투영
                FVector MoveDelta = FVector::DotProduct(DeltaWorld, AxisDirection) * AxisDirection;

                // 새 위치 설정
                SetWorldLocation(GetComponentLocation() + MoveDelta);

                // 상태 업데이트
                BeforeMousePos = MousePos;

                // 축 업데이트 이벤트 호출
                if (OnGizmoAxisUpdated.IsBound())
                {
                    OnGizmoAxisUpdated.Execute(this);
                }
            }

            if (!Viewport->KeyState(EKeys::LeftMouseButton) && bIsDragging)
            {
                OnMouseRelease();
                EditorViewportClient->bDisableInput = false;
                EditorViewportClient->LostFocus(EditorViewportClient->Viewport);
                EditorViewportClient->ReceivedFocus(EditorViewportClient->Viewport);
                
                FSlateApplication::Get().ResetToDefaultInputSettings();

                ActiveGizmo = nullptr;
            }
        }
    }
}

bool UGizmoTranslationComponent::DetectArrowHit(FVector MouseLocation, FVector MouseDirection, FHitResult& OutHit)
{
    struct ArrowData
    {
        UArrowComponent* ArrowComponent;
        FVector Start;
        FVector Direction;
        float Length;
        float Radius;
    };

    TArray<ArrowData> ArrowArray = {
        {XAxisArrow, XAxisArrow->GetComponentLocation(), XAxisArrow->GetForwardVector(), 100.0f, 10.0f},
        {YAxisArrow, YAxisArrow->GetComponentLocation(), YAxisArrow->GetForwardVector(), 100.0f, 10.0f},
        {ZAxisArrow, ZAxisArrow->GetComponentLocation(), ZAxisArrow->GetForwardVector(), 100.0f, 10.0f}
    };

    bool bHitDetected = false;
    float ClosestDistance = FLT_MAX;
    FHitResult TempHitResult;

    for (const ArrowData& Arrow : ArrowArray)
    {
        FVector ClosestPointOnArrow, ClosestPointOnLine;
        FMath::SegmentDistToSegmentSafe(
            Arrow.Start,
            Arrow.Start + Arrow.Direction * Arrow.Length,
            MouseLocation,
            MouseLocation + MouseDirection * 50000.0f,
            ClosestPointOnArrow,
            ClosestPointOnLine
        );

        float Distance = FVector::Distance(ClosestPointOnArrow, ClosestPointOnLine);
        if (Distance <= Arrow.Radius)
        {
            float ImpactDistance = FVector::Distance(MouseLocation, ClosestPointOnLine);
            if (ImpactDistance < ClosestDistance)
            {
                bHitDetected = true;
                ClosestDistance = ImpactDistance;

                // 가장 가까운 충돌 정보 TempHitResult에 저장
                TempHitResult.Component = Arrow.ArrowComponent;
                TempHitResult.ImpactPoint = ClosestPointOnLine;
                TempHitResult.Distance = ImpactDistance;
            }
        }
    }

    if (bHitDetected)
    {
        OutHit = TempHitResult;
        return true;
    }

    return false;
}

void UGizmoTranslationComponent::HandleRuntimeMode()
{
    if (bIsDragging && SelectedArrow)
    {
        FVector MouseLocation, MouseDirection;
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->DeprojectMousePositionToWorld(MouseLocation, MouseDirection))
        {
        }
    }
}

void UGizmoTranslationComponent::StartDragging(FVector HitLocation, UArrowComponent* HitArrow, FIntPoint MousePos)
{
    SelectedArrow = HitArrow;
    UpdateArrowColor(SelectedArrow, true);
    bIsDragging = true;
    BeforeMousePos = MousePos;
}

void UGizmoTranslationComponent::StopDragging()
{
    bIsDragging = false;
    UpdateArrowColor(SelectedArrow, false);
    SelectedArrow = nullptr;
}

void UGizmoTranslationComponent::OnMouseOver(UArrowComponent* HoveredArrow)
{
    if (bIsDragging)
    {
        UpdateArrowColor(SelectedArrow, true);
    }
    else
    {
        UpdateArrowColor(XAxisArrow, false);
        UpdateArrowColor(YAxisArrow, false);
        UpdateArrowColor(ZAxisArrow, false);

        if (HoveredArrow != nullptr)
        {
            UpdateArrowColor(HoveredArrow, true);
        }
    }
}

void UGizmoTranslationComponent::OnMouseClick(FVector HitLocation, UArrowComponent* HitArrow, FIntPoint MousePos)
{
    StartDragging(HitLocation, HitArrow, MousePos);
}

void UGizmoTranslationComponent::OnMouseRelease()
{
    StopDragging();
}

void UGizmoTranslationComponent::UpdateArrowColor(UArrowComponent* ArrowComponent, bool bIsSelected)
{
    if (ArrowComponent == XAxisArrow)
    {
        XAxisArrow->SetArrowColor(bIsSelected ? SelectedColor : DefaultXColor);
    }
    else if (ArrowComponent == YAxisArrow)
    {
        YAxisArrow->SetArrowColor(bIsSelected ? SelectedColor : DefaultYColor);
    }
    else if (ArrowComponent == ZAxisArrow)
    {
        ZAxisArrow->SetArrowColor(bIsSelected ? SelectedColor : DefaultZColor);
    }
}

EAxis::Type UGizmoTranslationComponent::GetAxisFromArrow(UArrowComponent* ArrowComponent) const
{
    if (ArrowComponent == XAxisArrow)
    {
        return EAxis::X;
    }
    else if (ArrowComponent == YAxisArrow)
    {
        return EAxis::Y;
    }
    else if (ArrowComponent == ZAxisArrow)
    {
        return EAxis::Z;
    }
    return EAxis::None;
}
