// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowBase.h"
#include "Templates/SharedPointer.h"
#include "ShowCamSequence.generated.h"

class UCameraComponent;
class USpringArmComponent;

// TODO: (DIPI) 추후 카메라 곡선 연출이 필요하면 아래 기능을 이용하도록 변경 필요
/*
* | 모드                    | 탄젠트 설정 필요 여부 | 설명                                                                                       
* |-------------------------|-----------------------|--------------------------------------------------------------------------------------------
* | CIM_Linear              | 탄젠트 무시           | 선형 보간으로, ArriveTangent와 LeaveTangent는 무시됩니다. 직선 경로를 생성합니다.            
* | CIM_CurveAuto           | 자동 설정             | 탄젠트를 자동으로 설정하여 부드러운 곡선을 만듭니다. 사용자가 직접 설정할 수 없습니다.       
* | CIM_Constant            | 탄젠트 무시           | 값이 일정하게 유지되다가 다음 키프레임에서 갑작스럽게 변경되므로, 탄젠트는 사용되지 않습니다.
* | CIM_CurveUser           | 수동 설정             | 탄젠트를 수동으로 설정할 수 있습니다. ArriveTangent와 LeaveTangent는 동일하게 맞춰집니다.    
* | CIM_CurveBreak          | 수동 설정             | 탄젠트를 수동으로 설정할 수 있으며, ArriveTangent와 LeaveTangent를 서로 독립적으로 설정합니다.
* | CIM_CurveAutoClamped    | 자동 설정             | 탄젠트를 자동으로 설정하며, 극단적인 값이 되지 않도록 클램핑하여 제한된 범위 내에서 설정합니다.
* | CIM_Unknown             | 무효                  | 유효하지 않은 값으로, 탄젠트를 설정해도 의미가 없습니다.                                   
* FInterpCurveVector / EInterpCurveMode
*/

//// FInterpCurve<FVector> 초기화
//FInterpCurve<FVector> PositionCurve;
//
//// 키프레임 추가 (순서대로)
//PositionCurve.AddPoint(0.0f, FVector(0, 0, 0), CIM_Unknown); // 0번 키프레임 (InVal, InterpMode : 사용안됨 의미 없는 값)
//PositionCurve.AddPoint(2.0f, FVector(100, 0, 0), CIM_Linear);   // 1번 키프레임
//PositionCurve.AddPoint(5.0f, FVector(200, 100, 0), CIM_CurveAutoClamped); // 2번 키프레임
//
//// 특정 시간에 따른 위치 계산
//float Time = CurrentTime;
//FVector Position = PositionCurve.Eval(Time, FVector::ZeroVector); // Time에 따른 보간된 위치

///** Float input value that corresponds to this key (eg. time). */
//float InVal;
//
///** Output value of templated type when input is equal to InVal. */
//T OutVal;
//
///** Tangent of curve arrive this point. */
//T ArriveTangent;
//
///** Tangent of curve leaving this point. */
//T LeaveTangent;
//
///** Interpolation mode between this point and the next one. @see EInterpCurveMode */
//TEnumAsByte<EInterpCurveMode> InterpMode;

// 카메라 연출 상태를 나타내는 Enum
UENUM(BlueprintType)
enum class ECameraSequenceState : uint8
{
    Wait,
    Playing,
    ReturningToStart,
    End,
};

// 카메라 연출 옵션
UENUM(BlueprintType)
enum class ECameraSequenceOption : uint8
{
    /** 캐릭터의 방향 기준으로 상대 좌표로 카메라 연출*/
    PlayerForward,

    /** 캐릭터의 방향은 무시하고 위치 기준으로 상대 좌표로 카메라 연출*/
    Player,

    /** 월드 좌표로 카메라 연출*/
    World,
    MAX,
};

// 카메라 종료 시 옵션
UENUM(BlueprintType)
enum class ECameraReturnOption : uint8
{
    /** 연출 시작할 때의 상태로 돌아감*/
    ReturnToStart,

    /** 카메라의 기본 위치값으로 돌아감*/
    ReturnToDefault,

    /** 종료될 때의 위치에서 원래의 타겟을 바라보도록*/
    MaintainEndPosition,
    MAX,
};

// 카메라 이동 방법을 나타내는 Enum
UENUM(BlueprintType)
enum class ECameraCurveMode : uint8
{
    /** 선형 보간
        A straight line between two keypoint values. */
    CIM_Linear = EInterpCurveMode::CIM_Linear,

    /** 탄젠트를 자동으로 설정하여 부드러운 곡선
        A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
        updated when points are moved, etc.  Tangents are unclamped and will plateau at curve start and end points. */
    CIM_CurveAuto = EInterpCurveMode::CIM_CurveAuto,

    /** 값이 일정하게 유지되다가 다음 키프레임에서 바로 변경 (워프)
        The out value is held constant until the next key, then will jump to that value. */
    CIM_Constant = EInterpCurveMode::CIM_Constant,

    /** 탄젠트를 자동으로 설정하며, 극단적인 값이 되지 않도록 클램핑하여 제한된 범위 내에서 설정
        A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
        updated when points are moved, etc.  Tangents are clamped and will plateau at curve start and end points. */
    CIM_CurveAutoClamped = EInterpCurveMode::CIM_CurveAutoClamped,

    /** Invalid or unknown curve type. */
    CIM_Unknown = EInterpCurveMode::CIM_Unknown,
};

// 카메라 이동 경로를 저장하는 구조체
USTRUCT(BlueprintType)
struct FCameraPathPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    ECameraCurveMode InterpMode = ECameraCurveMode::CIM_CurveAuto;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    float Duration = 0.3f; // 다음 포인트로 이동하는 데 걸리는 시간 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    FVector Position; // 카메라 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    FVector LookAtTarget; // 카메라의 룩앳 타겟 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    TOptional<float> FieldOfView; // 카메라 FOV

#if WITH_EDITOR
    bool bIsSelected = false;
    bool bNeedUpdateLocation = false;
#endif
};


USTRUCT(BlueprintType)
struct FShowCamSequenceKey : public FShowKey
{
    GENERATED_BODY()

    FShowCamSequenceKey()
    {
        KeyType = EShowKeyType::ShowKey_CamSequence;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    ECameraSequenceOption CameraSequenceOption = ECameraSequenceOption::PlayerForward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    ECameraReturnOption CameraReturnOption = ECameraReturnOption::ReturnToStart; // 연출이 끝난 후 시작 위치로 돌아갈지 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    ECameraCurveMode FadeInMode = ECameraCurveMode::CIM_CurveAuto; // 페이드 인 모드

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    float FadeOutBlendTime = 0.3f; // 페이드 아웃 시간 및 설정

    /**
    * If true, the tangents at the start and end points of the curve are set to zero, resulting in a "stationary" effect
    * at the endpoints. This means that the curve will smoothly start and stop at the beginning and end points.
    * If false, the tangents at the endpoints are calculated normally, allowing for smooth transitions beyond
    * the endpoints (useful for looping curves or when extending the curve’s influence).
    * 곡선의 시작점과 끝점에서 탄젠트를 0으로 설정할지 여부를 결정합니다.
    * true로 설정되면, 곡선이 시작점과 끝점에서 정지된 상태로 부드럽게 시작하고 멈추는 효과가 있습니다.
    * false로 설정되면, 시작점과 끝점의 탄젠트를 일반적으로 계산하여 곡선의 흐름이 끊기지 않고 이어지도록 합니다.
    * 이 설정은 루프가 아닌 곡선에서 자연스러운 시작과 끝을 표현할 때 유용합니다.
    */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    bool bStationaryEndpoints = false; // 페이드 아웃 시간 및 설정

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    TArray<FCameraPathPoint> PathPoints; // 카메라 이동 경로 리스트
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowCamSequence : public UShowBase
{
	GENERATED_BODY()

public:
    virtual FString GetTitle() override;
    virtual float GetLength() override;
    const FShowCamSequenceKey* GetShowCamSequenceKeyPtr() const { return ShowCamSequenceKeyPtr; }

protected:
    virtual void Initialize() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Reset();
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) override;
    virtual void ApplyTimeScale(float FinalTimeScale) override {};

private:
    void AddVectorPoint(FInterpCurve<FVector>& Curve, const float InVal, const FVector& OutVal, const ECameraCurveMode Mode);
    FVector CalculateRelativePositionForPlayback(const FVector& OriginVactor, const FVector& ActorWorldPosition, const FRotator& ActorWorldRotation, ECameraSequenceOption Option) const;
    
    void ApplyCameraSettings(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV);
    void ApplyRunTimeCamera(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV);
#if WITH_EDITOR
    void ApplyEditorViewCamera(const FVector& NewPosition, const FVector& NewLookAt, TOptional<float>& NewFOV);
#endif
    //void PathPointPlay(AActor* OwnerActor, float DeltaTime);
    void PathPointPlay(AActor* OwnerActor, float BasePassedTime);
    void PathPointReturningToStart(AActor* OwnerActor, float DeltaTime);

    FVector GetCameraLocation();
    FVector GetCameraLookAt();
    float GetCameraFOV();

private:
    const FShowCamSequenceKey* ShowCamSequenceKeyPtr = nullptr;

    UPROPERTY()
    TObjectPtr<APlayerController> PlayerController = nullptr;

    UPROPERTY()
    TObjectPtr<UCameraComponent> CameraComponent = nullptr;
    
    UPROPERTY()
    TObjectPtr<USpringArmComponent> SpringArmComponent = nullptr;

    ECameraSequenceState State = ECameraSequenceState::Wait;

    // 연출 시작 시 카메라 위치 LookAt 캐릭터 상대 좌표로 저장, FOV저장, 스프링암의 UsePawnControl 저장
    FVector InitialRelativeLocationFromSocket;
    FVector InitialRelativeLookAtFromSocket;
    float InitialFOV;
    bool bInitialUsePawnControlRotation = false;

    float CurrentBlendTime = 0.0f;
    int32 CurrentPointIndex = 0;

    // 이전 프레임의 카메라 위치와 LookAt
    FVector PreviousLocation;
    FVector PreviousLookAt;


    float PlaybackEndTime = 0.0f;
    FInterpCurve<FVector> PositionCurve;
    FInterpCurve<FVector> LookAtCurve;
    FInterpCurve<float> FOVCurve;
};
