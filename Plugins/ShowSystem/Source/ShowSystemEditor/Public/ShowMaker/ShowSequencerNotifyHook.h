// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "Misc/NotifyHook.h"

/**
 * 
 */
class SHOWSYSTEMEDITOR_API ShowSequencerNotifyHook : public FNotifyHook
{
public:
    ShowSequencerNotifyHook(TObjectPtr<UShowSequencer> InShowSequencer)
    {
        ShowSequencer = InShowSequencer;
    }

    virtual ~ShowSequencerNotifyHook()
    {
        if (ShowSequencer)
		{
			ShowSequencer = nullptr;
		}
    }

    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged) override
    {
        UE_LOG(LogTemp, Log, TEXT("ShowSequencerNotifyHook::NotifyPostChange"));
        MarkPackageDirty();
    }

    void MarkPackageDirty()
    {
        if (ShowSequencer)
        {
            ShowSequencer->MarkPackageDirty();
        }
    }

private:
    TObjectPtr<UShowSequencer> ShowSequencer = nullptr;
    TSharedPtr<IStructureDetailsView> StructureDetailsView = nullptr;
};
