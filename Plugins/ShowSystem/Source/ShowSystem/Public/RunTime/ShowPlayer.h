// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShowPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowPlayer : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
    void PlaySoloShow(AActor* Owner, class UShowSequencer* ShowSequencer);

    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
    void StopSoloShow(AActor* Owner, UShowSequencer* ShowSequencer);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;

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

private:
	bool Initialized = false;
	//TSparseArray<TObjectPtr<UShowSequencer>> ShowSequencers;
};
