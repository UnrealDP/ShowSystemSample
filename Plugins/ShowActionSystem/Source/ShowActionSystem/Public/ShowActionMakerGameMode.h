// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShowActionMakerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEM_API AShowActionMakerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
    AShowActionMakerGameMode();

protected:
    virtual void BeginPlay() override;

public:
    // Enabling Tick for GameMode
    virtual void Tick(float DeltaSeconds) override;
};
