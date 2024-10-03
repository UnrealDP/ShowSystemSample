// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShowActionMakerGameMode.generated.h"

struct FSkillData;
struct FSkillShowData;

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEMEDITOR_API AShowActionMakerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
    AShowActionMakerGameMode();

private:
    void GetPos(FVector& CasterPos, FVector& TargetPos, FRotator& TargetRotator);
    void SaveActorPositions();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);

public:
    // Enabling Tick for GameMode
    virtual void Tick(float DeltaSeconds) override;

    void SelectAction(FName InSelectedActionName, FSkillData* InSkillData);
    void DoAction();    

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowActionMakerGameMode")
    TSubclassOf<AActor> DefaultActorClass = nullptr;
    
private:
    TObjectPtr<AActor> Caster = nullptr;
    TArray<TObjectPtr<AActor>> Targets;

    FName SelectedActionName;
    FSkillData* SkillData = nullptr;
};
