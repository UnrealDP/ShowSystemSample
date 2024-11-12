// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "ShowSequenceCamShakePattern.generated.h"

struct FMinimalViewInfo;

class UCameraAnimationSequenceCameraStandIn;
class UMovieSceneEntitySystemLinker;
class UMovieSceneSequence;
class UCameraAnimationSequencePlayer;

/**
 * A camera shake pattern that plays a sequencer animation.
 */
UCLASS()
class SHOWSYSTEM_API UShowSequenceCamShakePattern : public UCameraShakePattern
{
public:

	GENERATED_BODY()

	UShowSequenceCamShakePattern(const FObjectInitializer& ObjInit);

public:

	/** Source camera animation sequence to play. */
	UPROPERTY(EditAnywhere, Category = CameraShake)
	TObjectPtr<class UCameraAnimationSequence> Sequence;

	/** Scalar defining how fast to play the anim. */
	UPROPERTY(EditAnywhere, Category = CameraShake, meta = (ClampMin = "0.001"))
	float PlayRate;

	/** Scalar defining how "intense" to play the anim. */
	UPROPERTY(EditAnywhere, Category = CameraShake, meta = (ClampMin = "0.0"))
	float Scale;

	/** Linear blend-in time. */
	UPROPERTY(EditAnywhere, Category = CameraShake, meta = (ClampMin = "0.0"))
	float BlendInTime;

	/** Linear blend-out time. */
	UPROPERTY(EditAnywhere, Category = CameraShake, meta = (ClampMin = "0.0"))
	float BlendOutTime;

	/** When bRandomSegment is true, defines how long the sequence should play. */
	UPROPERTY(EditAnywhere, Category = CameraShake, meta = (ClampMin = "0.0", EditCondition = "bRandomSegment"))
	float RandomSegmentDuration;

	/**
	 * When true, plays a random snippet of the sequence for RandomSegmentDuration seconds.
	 *
	 * @note The sequence we be forced to loop when bRandomSegment is enabled, in case the duration
	 *       is longer than what's left to play from the random start time.
	 */
	UPROPERTY(EditAnywhere, Category = CameraShake)
	bool bRandomSegment;

private:

	// UCameraShakeBase interface
	virtual void GetShakePatternInfoImpl(FCameraShakeInfo& OutInfo) const override;
	virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override;
	virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual void ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult) override;
	virtual bool IsFinishedImpl() const override;
	virtual void StopShakePatternImpl(const FCameraShakePatternStopParams& Params) override;
	virtual void TeardownShakePatternImpl() override;

	void UpdateCamera(FFrameTime NewPosition, const FMinimalViewInfo& InPOV, FCameraShakePatternUpdateResult& OutResult);

public:
	void UpdatePlayRate(float NewPlayRate);

private:

	/** The player we use to play the camera animation sequence */
	UPROPERTY(Instanced, Transient)
	TObjectPtr<UCameraAnimationSequencePlayer> Player;

	/** Standin for the camera actor and components */
	UPROPERTY(Instanced, Transient)
	TObjectPtr<UCameraAnimationSequenceCameraStandIn> CameraStandIn;

	/** State tracking */
	FCameraShakeState State;
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CameraAnimationSequence.h"
#include "CineCameraComponent.h"
#include "EntitySystem/MovieSceneEntityIDs.h"
#endif