// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShowActionMakerGameMode.generated.h"

struct FSkillData;
struct FSkillShowData;
class UActionBase;

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

    UActionBase* SelectAction(FName InSelectedActionName, FSkillData* InSkillData, FSkillShowData* InSkillShowData);
    void DoAction();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShowActionMakerGameMode")
    TSubclassOf<AActor> DefaultActorClass = nullptr;
    
private:
    TObjectPtr<AActor> Caster = nullptr;
    TArray<TObjectPtr<AActor>> Targets;

    FName SelectedActionName;
    FSkillData* SkillData = nullptr;
    FSkillShowData* SkillShowData = nullptr;

public:
    UActionBase* CrrActionPtr = nullptr;
};



