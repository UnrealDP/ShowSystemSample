// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowBase.h"
#include "ShowCamSequence.generated.h"

class UCameraComponent;
class USpringArmComponent;

// 카메라 연출 상태를 나타내는 Enum
UENUM(BlueprintType)
enum class ECameraSequenceState : uint8
{
    Wait,               // 연출이 실행되지 않음
    Playing,            // 연출 중
    ReturningToStart,   // 시작 위치로 돌아가는 중
    End,                // 연출 종료
};

// 카메라 이동 경로를 저장하는 구조체
USTRUCT(BlueprintType)
struct FCameraPathPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    FVector Position; // 카메라 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    FVector LookAtTarget; // 카메라의 룩앳 타겟 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    float Duration = 0.3f; // 다음 포인트로 이동하는 데 걸리는 시간 (초)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Path")
    TOptional<float> FieldOfView; // 카메라 FOV
};


USTRUCT(BlueprintType)
struct FShowCamSequenceKey : public FShowKey
{
    GENERATED_BODY()

    FShowCamSequenceKey()
    {
        KeyType = EShowKeyType::ShowKey_CamSequence;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    TArray<FCameraPathPoint> PathPoints; // 카메라 이동 경로 리스트

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    bool bReturnToStartPosition = true; // 연출이 끝난 후 시작 위치로 돌아갈지 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Sequence")
    float FadeOutBlendTime = 0.3f; // 페이드 아웃 시간 및 설정
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

protected:
    virtual void Initialize() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Reset();
    virtual void Pause() override {};
    virtual void UnPause() override {};

private:
    virtual void Tick(float DeltaTime, float BasePassedTime) override;
    void ApplyCameraSettings(const FVector& NewPosition, const FRotator& NewRotation, TOptional<float>& NewFOV);
    void PathPointPlay(AActor* OwnerActor);
    void PathPointReturningToStart(AActor* OwnerActor);

    FVector GetCameraLocation();
    FVector GetCameraLookAt(float Distance = 1000.0f);
    void ApplyCharacterOffsetToSpringArm();

private:
    const FShowCamSequenceKey* ShowCamSequenceKeyPtr = nullptr;

    UPROPERTY()
    TObjectPtr<APlayerController> PlayerController = nullptr;

    UPROPERTY()
    TObjectPtr<UCameraComponent> CameraComponent = nullptr;
    
    UPROPERTY()
    TObjectPtr<USpringArmComponent> SpringArmComponent = nullptr;

    ECameraSequenceState State = ECameraSequenceState::Wait;

    // 연출 시작 시의 캐릭터 위치 저장
    FVector InitialLocalCharacterLocation;
    FRotator InitialLocalCharacterRotation;

    // 연출 시작 시의 카메라 위치 저장
    FVector InitialCameraRelativePosition;
    FVector InitialLookAtRelativeTarget;
    float InitialFOV;

    float CurrentBlendTime = 0.0f;
    int32 CurrentPointIndex = 0;

    float InitialTargetArmLength = 0.0f;
    FVector InitialCameraRelativePositionForBlend;
    FVector InitialLookAtRelativeTargetForBlend;
    float InitialFOVForBlend;
};