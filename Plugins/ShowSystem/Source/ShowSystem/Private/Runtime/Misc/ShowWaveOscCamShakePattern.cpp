// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/Misc/ShowWaveOscCamShakePattern.h"


UShowWaveOscCamShakePattern::UShowWaveOscCamShakePattern(const FObjectInitializer& ObjInit)
	: Super(ObjInit)
{
	// Default to only location shaking.
	RotationAmplitudeMultiplier = 0.f;
	FOV.Amplitude = 0.f;
}

void UShowWaveOscCamShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
	Super::StartShakePatternImpl(Params);

	if (!Params.bIsRestarting)
	{
		X.Initialize(InitialLocationOffset.X);
		Y.Initialize(InitialLocationOffset.Y);
		Z.Initialize(InitialLocationOffset.Z);

		CurrentLocationOffset = InitialLocationOffset;

		Pitch.Initialize(InitialRotationOffset.X);
		Yaw.Initialize(InitialRotationOffset.Y);
		Roll.Initialize(InitialRotationOffset.Z);

		CurrentRotationOffset = InitialRotationOffset;

		FOV.Initialize(InitialFOVOffset);

		CurrentFOVOffset = InitialFOVOffset;
	}
}

void UShowWaveOscCamShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	float ApplyDeltaTime = bIsPause ? 0.f : Params.DeltaTime * PlayRate;
	UpdateOscillators(ApplyDeltaTime, OutResult);

	const float BlendWeight = State.Update(ApplyDeltaTime);
	OutResult.ApplyScale(BlendWeight);
}

void UShowWaveOscCamShakePattern::ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult)
{
	// Scrubbing is like going back to our initial state and updating directly to the scrub time.
	CurrentLocationOffset = InitialLocationOffset;
	CurrentRotationOffset = InitialRotationOffset;
	CurrentFOVOffset = InitialFOVOffset;

	UpdateOscillators(Params.AbsoluteTime, OutResult);

	const float BlendWeight = State.Scrub(Params.AbsoluteTime);
	OutResult.ApplyScale(BlendWeight);
}

void UShowWaveOscCamShakePattern::UpdateOscillators(float DeltaTime, FCameraShakePatternUpdateResult& OutResult)
{
	OutResult.Location.X = X.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.X);
	OutResult.Location.Y = Y.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.Y);
	OutResult.Location.Z = Z.Update(DeltaTime, LocationAmplitudeMultiplier, LocationFrequencyMultiplier, CurrentLocationOffset.Z);

	OutResult.Rotation.Pitch = Pitch.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.X);
	OutResult.Rotation.Yaw = Yaw.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.Y);
	OutResult.Rotation.Roll = Roll.Update(DeltaTime, RotationAmplitudeMultiplier, RotationFrequencyMultiplier, CurrentRotationOffset.Z);

	OutResult.FOV = FOV.Update(DeltaTime, 1.f, 1.f, CurrentFOVOffset);
}