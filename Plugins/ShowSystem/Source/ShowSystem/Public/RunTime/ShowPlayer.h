// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShowPlayer.generated.h"

class UShowSequencer;

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowPlayer : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
	void PlaySoloShow(AActor* Owner, UShowSequencer* ShowSequencerPtr);

    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
	void ResetSoloShow(AActor* Owner, UShowSequencer* ShowSequencerPtr);

	UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
	void DisposeSoloShow(AActor* Owner, UShowSequencer* ShowSequencerPtr);

	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void Initialize();

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

	UShowSequencer* NewShowSequencer(AActor* Owner, const FSoftObjectPath& ShowPath);

	/**
	 * Pure virtual that must be overloaded by the inheriting class. It will
	 * be called from within LevelTick.cpp after ticking all actors or from
	 * the rendering thread (depending on bIsRenderingThreadObject)
	 *
	 * @param DeltaTime	Game time passed since the last call.
	 */
    virtual void Tick(float DeltaTime) override;

	/** return the stat id to use for this tickable **/
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UShowPlayer, STATGROUP_Tickables);
	}

	bool HasShowSequencer(const AActor* Owner, const UShowSequencer* SequencerPtr) const;

private:
	bool Initialized = false;
};
