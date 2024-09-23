// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShowSequencerComponent.generated.h"


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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Show")
    int32 PlayShow(class UShowSequencer* InShowSequencer);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void StopShow(int32 ID);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void DisposeShow(int32 ID);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void PauseShow(int32 ID);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void UnPauseShow(int32 ID);

    UFUNCTION(BlueprintCallable, Category = "Show")
    void ChangeSpeedShow(int32 ID, float Speed);

private:
    bool bIsPlaying = true;
    TSparseArray<TObjectPtr<UShowSequencer>> ShowSequencers; // 연출 시퀀스 트래킹
};
