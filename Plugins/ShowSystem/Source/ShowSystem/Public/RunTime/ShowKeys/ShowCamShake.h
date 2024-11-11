// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "RunTime/Misc/ShowPerlinNoiseCamShakePattern.h"
#include "RunTime/Misc/ShowSequenceCamShakePattern.h"
#include "SequenceCameraShake.h"
#include "ShowCamShake.generated.h"

class UCharacterMovementComponent;

UENUM()
enum class ECameraShakePattern
{
	PerlinNoise,
	Sequence,
};

USTRUCT(BlueprintType)
struct FShowCameraShake
{
    GENERATED_BODY()

    TSubclassOf<UCameraShakePattern> CameraShakePattern;
};

USTRUCT(BlueprintType)
struct FShowPerlinNoiseCameraShake : public FShowCameraShake
{
    GENERATED_BODY()

    FShowPerlinNoiseCameraShake()
    {
        CameraShakePattern = UShowPerlinNoiseCamShakePattern::StaticClass();
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 1.f;

    /** Blend-in time for this shake. Zero or less means no blend-in. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendInTime = 0.2f;

    /** Blend-out time for this shake. Zero or less means no blend-out. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BlendOutTime = 0.2f;

    /** 모든 위치 진동에 적용되는 진폭 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    float LocationAmplitudeMultiplier = 1.f;

    /** 모든 위치 진동에 적용되는 주파수 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    float LocationFrequencyMultiplier = 1.f;

    /** Shake in the X axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    FPerlinNoiseShaker X;

    /** Shake in the Y axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    FPerlinNoiseShaker Y;

    /** Shake in the Z axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Location")
    FPerlinNoiseShaker Z;

    /** 모든 회전 진동에 적용되는 진폭 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    float RotationAmplitudeMultiplier = 1.f;

    /** 모든 회전 진동에 적용되는 주파수 배율 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    float RotationFrequencyMultiplier = 1.f;

    /** Pitch shake. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FPerlinNoiseShaker Pitch;

    /** Yaw shake. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FPerlinNoiseShaker Yaw;

    /** Roll shake. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
    FPerlinNoiseShaker Roll;

    /** FOV shake. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FOV")
    FPerlinNoiseShaker FOV;
};

USTRUCT(BlueprintType)
struct FShowSequenceCameraShake : public FShowCameraShake
{
    GENERATED_BODY()

    FShowSequenceCameraShake()
    {
        CameraShakePattern = UShowSequenceCamShakePattern::StaticClass();
    }

    /** 재생할 카메라 애니메이션 시퀀스 소스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<class UCameraAnimationSequence> Sequence = nullptr;

    /** 애니메이션 재생 속도를 정의하는 스칼라 값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.001"))
    float PlayRate = 1.0f;

    /** 애니메이션 재생 강도를 정의하는 스칼라 값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float Scale = 1.0f;

    /** 블렌드-인 시간 (애니메이션 시작 시 서서히 강해지는 시간) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float BlendInTime = 0.2f;

    /** 블렌드-아웃 시간 (애니메이션 종료 시 서서히 약해지는 시간) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float BlendOutTime = 0.2f;

    /** bRandomSegment가 true일 때, 시퀀스가 재생되는 시간을 정의 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", EditCondition = "bRandomSegment"))
    float RandomSegmentDuration;

    /**
     * true로 설정되면 시퀀스의 랜덤한 구간을 선택하여 RandomSegmentDuration 동안 재생합니다.
     *
     * @note bRandomSegment가 활성화되면 남은 재생 시간이 부족할 경우 시퀀스가 강제로 루프됩니다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRandomSegment;
};


USTRUCT(BlueprintType)
struct FShowCamShakeKey : public FShowKey
{
    GENERATED_BODY()

    FShowCamShakeKey()
    {
        KeyType = EShowKeyType::ShowKey_CamShake;
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECameraShakePattern CameraShakePattern = ECameraShakePattern::PerlinNoise;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::World;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator UserPlaySpaceRot = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSingleInstance = true;    

    // 카메라 쉐이크 패턴 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FInstancedStruct PatternData;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowCamShake : public UShowBase
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
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) override;
    virtual void ApplyTimeScale(float FinalTimeScale) override;

private:

private:
    const FShowCamShakeKey* CamShakeKeyPtr = nullptr;

    UPROPERTY()
    TObjectPtr<UCameraShakeBase> CameraShakeInstance;

    bool bBeforeMoving;
};
