// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShowSequencerComponent.generated.h"

class UShowSequencer;

UCLASS( BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOWSYSTEM_API UShowSequencerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShowSequencerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Show")
    void PlayShow(UShowSequencer* InShowSequencer);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void StopShow(UShowSequencer* InShowSequencer) const;

    UFUNCTION(BlueprintCallable, Category = "Show")
    void DisposeShow(UShowSequencer* InShowSequencer);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void PauseShow(UShowSequencer* InShowSequencer) const;

    UFUNCTION(BlueprintCallable, Category = "Show")
    void UnPauseShow(UShowSequencer* InShowSequencer) const;

    UFUNCTION(BlueprintCallable, Category = "Show")
    void ChangeShowTimeScalse(UShowSequencer* InShowSequencer, float InTimeScale) const;

private:
    bool bIsPlaying = true;
    TArray<TObjectPtr<UShowSequencer>> ShowSequencers; // 연출 시퀀스 트래킹
};
