// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/Misc/ShowPerlinNoiseCamShakePattern.h"

UShowPerlinNoiseCamShakePattern::UShowPerlinNoiseCamShakePattern(const FObjectInitializer& ObjInit)
	: Super(ObjInit)
{
	// Default to only location shaking.
	RotationAmplitudeMultiplier = 0.f;
	FOV.Amplitude = 0.f;
}

void UShowPerlinNoiseCamShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	Super::StartShakePatternImpl(Params);

	if (!Params.bIsRestarting)
	{
		// All offsets are random. This is because the core perlin noise implementation
		// uses permutation tables, so if two shakers have the same initial offset and the same
		// frequency, they will have the same exact values.
		InitialLocationOffset = FVector3f((float)FMath::RandHelper(255), (float)FMath::RandHelper(255), (float)FMath::RandHelper(255));
		InitialRotationOffset = FVector3f((float)FMath::RandHelper(255), (float)FMath::RandHelper(255), (float)FMath::RandHelper(255));
		InitialFOVOffset = (float)FMath::RandHelper(255);

		CurrentLocationOffset = InitialLocationOffset;
		CurrentRotationOffset = InitialRotationOffset;
		CurrentFOVOffset = InitialFOVOffset;
	}
}

void UShowPerlinNoiseCamShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	float ApplyDeltaTime = bIsPause ? 0.f : Params.DeltaTime * PlayRate;
	UpdatePerlinNoise(ApplyDeltaTime, OutResult);

	const float BlendWeight = State.Update(ApplyDeltaTime);
	OutResult.ApplyScale(BlendWeight);
}

void UShowPerlinNoiseCamShakePattern::ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	// Scrubbing is like going back to our initial state and updating directly to the scrub time.
	CurrentLocationOffset = InitialLocationOffset;
	CurrentRotationOffset = InitialRotationOffset;
	CurrentFOVOffset = InitialFOVOffset;

	UpdatePerlinNoise(Params.AbsoluteTime, OutResult);

	const float BlendWeight = State.Scrub(Params.AbsoluteTime);
	OutResult.ApplyScale(BlendWeight);
}

void UShowPerlinNoiseCamShakePattern::UpdatePerlinNoise(float DeltaTime, FCameraShakePatternUpdateResult& OutResult)
{
	OutResult.Location.X = X.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.X);
	OutResult.Location.Y = Y.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.Y);
	OutResult.Location.Z = Z.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.Z);

	OutResult.Rotation.Pitch = Pitch.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.X);
	OutResult.Rotation.Yaw = Yaw.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.Y);
	OutResult.Rotation.Roll = Roll.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.Z);

	OutResult.FOV = FOV.Update(DeltaTime, 1.f, 1.f, CurrentFOVOffset);
}