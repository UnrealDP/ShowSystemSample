// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShowActionMakerGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEMEDITOR_API AShowActionMakerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
    AShowActionMakerGameMode();

protected:
    virtual void BeginPlay() override;

public:
    // Enabling Tick for GameMode
    virtual void Tick(float DeltaSeconds) override;

public:
    UPROPERTY(EditAnywhere, Category = "ShowActionMakerGameMode")
    TObjectPtr<AActor> DefaultActor;
    
private:
    TObjectPtr<AActor> Caster;
    TArray<TObjectPtr<AActor>> Targets;
};
