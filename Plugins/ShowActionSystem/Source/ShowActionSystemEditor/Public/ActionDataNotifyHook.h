// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_ThreeParams(FPropertyActionDataChanged, const FPropertyChangedEvent&, FEditPropertyChain*, FSkillData*)

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API ActionDataNotifyHook : public FNotifyHook
{
public:
	ActionDataNotifyHook(UDataTable* InLoadedSkillDataTable, FPropertyActionDataChanged InOnPropertyChanged)
    {
		LoadedSkillDataTable = InLoadedSkillDataTable;
        OnPropertyChanged = InOnPropertyChanged;
    }
	~ActionDataNotifyHook()
    {
    }

    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged) override
    {
        LoadedSkillDataTable->MarkPackageDirty();
        if (OnPropertyChanged.IsBound())
		{
			OnPropertyChanged.Execute(PropertyChangedEvent, PropertyThatChanged, CrrSkillData);
		}
    }

    UDataTable* LoadedSkillDataTable;
    FPropertyActionDataChanged OnPropertyChanged;
    FSkillData* CrrSkillData = nullptr;
};
