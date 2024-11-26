// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/WaveOscillatorCameraShakePattern.h"
#include "ShowWaveOscCamShakePattern.generated.h"

/**
 * A camera shake that uses oscillations to move the camera.
 */
UCLASS(meta = (AutoExpandCategories = "Location,Rotation,FOV,Timing"))
class SHOWSYSTEM_API UShowWaveOscCamShakePattern : public USimpleCameraShakePattern
{
public:

	GENERATED_BODY()

	UShowWaveOscCamShakePattern(const FObjectInitializer& ObjInit);

public:

	/** Amplitude multiplier for all location oscillation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	float LocationAmplitudeMultiplier = 1.f;

	/** Frequency multiplier for all location oscillation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	float LocationFrequencyMultiplier = 1.f;

	/** Oscillation in the X axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FWaveOscillator X;

	/** Oscillation in the Y axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FWaveOscillator Y;

	/** Oscillation in the Z axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Location)
	FWaveOscillator Z;

	/** Amplitude multiplier for all rotation oscillation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	float RotationAmplitudeMultiplier = 1.f;

	/** Frequency multiplier for all rotation oscillation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	float RotationFrequencyMultiplier = 1.f;

	/** Pitch oscillation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FWaveOscillator Pitch;

	/** Yaw oscillation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FWaveOscillator Yaw;

	/** Roll oscillation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rotation)
	FWaveOscillator Roll;

	/** FOV oscillation. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FOV)
	FWaveOscillator FOV;

private:

	// UCameraShakePattern interface
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult) override;

	void UpdateOscillators(float DeltaTime, FCameraShakePatternUpdateResult& OutResult);

private:

	/** Initial sinusoidal offset for location oscillation. */
	FVector3f InitialLocationOffset;
	/** Current sinusoidal offset for location oscillation. */
	FVector3f CurrentLocationOffset;

	/** Initial sinusoidal offset for rotation oscillation. */
	FVector3f InitialRotationOffset;
	/** Current sinusoidal offset for rotation oscillation. */
	FVector3f CurrentRotationOffset;

	/** Initial sinusoidal offset for FOV oscillation */
	float InitialFOVOffset;
	/** Current sinusoidal offset for FOV oscillation */
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
