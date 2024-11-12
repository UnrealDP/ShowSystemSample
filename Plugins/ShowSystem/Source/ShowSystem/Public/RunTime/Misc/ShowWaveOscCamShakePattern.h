// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/WaveOscillatorCameraShakePattern.h"
#include "ShowWaveOscCamShakePattern.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowWaveOscCamShakePattern : public UWaveOscillatorCameraShakePattern
{
	GENERATED_BODY()
	
public:
	void UpdateDuration(float NewDuration)
	{
		const FCameraShakeInfo& CameraShakeInfo = State.GetShakeInfo();
		FCameraShakeInfo& NonConstCameraShakeInfo = const_cast<FCameraShakeInfo&>(CameraShakeInfo);
		NonConstCameraShakeInfo.Duration = FCameraShakeDuration(NewDuration);
	}
};
