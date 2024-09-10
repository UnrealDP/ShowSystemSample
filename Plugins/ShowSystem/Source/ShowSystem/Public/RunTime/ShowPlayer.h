// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShowPlayer.generated.h"

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowPlayer : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
    void PlaySoloShow(AActor* Owner, class UShowSequencer* ShowSequencer);

    UFUNCTION(BlueprintCallable, Category = "ShowPlayer")
    void StopSoloShow(AActor* Owner, UShowSequencer* ShowSequencer);

private:
    TMap<AActor*, UShowSequencer*> ActiveShows;
};
