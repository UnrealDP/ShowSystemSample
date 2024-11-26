// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"
#include "Shakes/SimpleCameraShakePattern.h"
#include "ShowPerlinNoiseCamShakePattern.generated.h"

/**
/**
 * A camera shake that uses Perlin noise to shake the camera.
 */
UCLASS(meta = (AutoExpandCategories = "Location,Rotation,FOV,Timing"))
class SHOWSYSTEM_API UShowPerlinNoiseCamShakePattern : public USimpleCameraShakePattern
{
public:

	GENERATED_BODY()

	UShowPerlinNoiseCamShakePattern(const FObjectInitializer& ObjInit);

public:

	/** Amplitude multiplier for all location shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	float LocationAmplitudeMultiplier = 1.f;

	/** Frequency multiplier for all location shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	float LocationFrequencyMultiplier = 1.f;

	/** Shake in the X axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FPerlinNoiseShaker X;

	/** Shake in the Y axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FPerlinNoiseShaker Y;

	/** Shake in the Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FPerlinNoiseShaker Z;

	/** Amplitude multiplier for all rotation shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	float RotationAmplitudeMultiplier = 1.f;

	/** Frequency multiplier for all rotation shake */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	float RotationFrequencyMultiplier = 1.f;

	/** Pitch shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FPerlinNoiseShaker Pitch;

	/** Yaw shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FPerlinNoiseShaker Yaw;

	/** Roll shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FPerlinNoiseShaker Roll;

	/** FOV shake. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FOV)
	FPerlinNoiseShaker FOV;

private:

	// UCameraShakePattern interface
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult) override;

	void UpdatePerlinNoise(float DeltaTime, FCameraShakePatternUpdateResult& OutResult);

private:

	/** Initial perlin noise offset for location oscillation. */
	FVector3f InitialLocationOffset;
	/** Current perlin noise offset for location oscillation. */
	FVector3f CurrentLocationOffset;

	/** Initial perlin noise offset for rotation oscillation. */
	FVector3f InitialRotationOffset;
	/** Current perlin noise offset for rotation oscillation. */
	FVector3f CurrentRotationOffset;

	/** Initial perlin noise offset for FOV oscillation */
	float InitialFOVOffset;
	/** Current perlin noise offset for FOV oscillation */
	float CurrentFOVOffset;

public:
	void UpdatePlayRate(float InPlayRate)
	{
		PlayRate = InPlayRate;
	}
	void Pause()
	{
		bIsPause = true;
	}
	void UnPause()
	{
		bIsPause = false;
	}

private:
	float PlayRate = 1.0f;
	bool bIsPause = false;
};
