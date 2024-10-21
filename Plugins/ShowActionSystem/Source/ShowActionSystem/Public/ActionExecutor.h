// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionBase.h"
#include "ActionExecutor.generated.h"

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEM_API UActionExecutor : public UObject, public FTickableGameObject
{
    GENERATED_BODY()

private:
    UActionExecutor() {}
    virtual void BeginDestroy() override;

public:
    UActionExecutor(UActionBase* InActionBasePtr)
    {
        checkf(InActionBasePtr != nullptr, TEXT("UActionExecutor::UActionExecutor InActionBasePtr is invalid"));
        ActionBasePtr = InActionBasePtr;
    }

    /**
     * Used to determine whether the object should be ticked in the editor.  Defaults to false since
     * that is the previous behavior.
     *
     * @return	true if this tickable object can be ticked in the editor
     */
    virtual bool IsTickableInEditor() const
    {
        return true;
    }

    // -- Start FTickableGameObject virtual = 0;
    virtual void Tick(float DeltaTime) override;

    /** return the stat id to use for this tickable **/
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(UActionExecutor, STATGROUP_Tickables);
    }
    // -- End FTickableGameObject virtual = 0;

private:
    UActionBase* ActionBasePtr;

    float PassedTime = 0.0f;
    float StepPassedTime = 0.0f;
};
