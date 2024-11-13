// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Data/ActionBaseData.h"
#include "ShowActionMakerGameMode.generated.h"

struct FSkillData;
struct FSkillShowData;
class UActionBase;
class UActionServerExecutor;
class UShowSequencer;
class ADebugCameraHelper;
class UShowBase;
class FShowSequencerEditorHelper;
struct FShowCamSequenceKey;
struct FCameraPathPoint;

DECLARE_DELEGATE_ThreeParams(FOnUpdateCameraView, APawn*, FCameraPathPoint*, const TArray<AActor*>&);

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEMEDITOR_API AShowActionMakerGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
    AShowActionMakerGameMode();
    void Initialize(FOnUpdateCameraView InOnUpdateCameraView);

private:
    void GetPos(FVector& CasterPos, FVector& TargetPos, FRotator& CasterRotator, FRotator& TargetRotator);
    void SaveActorPositions();
    virtual void Tick(float DeltaSeconds) override;
    void ShowSequenceAssetMarkPackageDirty();

    UFUNCTION()
    void OnActorDestroyed(AActor* DestroyedActor);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
    
    void OnMouseLClick();
    FVector GetCameraLocation();
    void UpdateCameraPathPoint(FCameraPathPoint* CameraPathPoint);
    void NotifyShowKeyChange(UShowBase* ShowBasePtr, FName PropertyName);

public:

    void SelectKey(TSharedPtr<FShowSequencerEditorHelper> InSelectedShowSequencerEditorHelper, UShowBase* InSelectedShowBasePtr);
    UActionBase* SelectAction(
        FName InSelectedActionName, 
        FSkillData* InSkillData, 
        FSkillShowData* InSkillShowData,
        UShowSequencer*& OutCastShowSequencer,
        UShowSequencer*& OutExecShowSequencer,
        UShowSequencer*& OutFinishShowSequencer);

    void DisposeAction();
    UShowSequencer* ChangeShow(EActionState ActionState, FSoftObjectPath* NewShowPath);
    void DoAction();    
    void ChangeTimeScale(float scale);

private:

    APawn* Caster = nullptr;

    TArray<APawn*> Targets;

    FName SelectedActionName;
    FSkillData* SkillData = nullptr;
    FSkillShowData* SkillShowData = nullptr;

    ADebugCameraHelper* DebugCameraHelper = nullptr;

    TSharedPtr<FShowSequencerEditorHelper> SelectedShowSequencerEditorHelper = nullptr;
    UShowBase* SelectedShowBasePtr = nullptr;

public:
    UActionServerExecutor* CrrActionPtr = nullptr;
    FOnUpdateCameraView OnUpdateCameraView = nullptr;
};



