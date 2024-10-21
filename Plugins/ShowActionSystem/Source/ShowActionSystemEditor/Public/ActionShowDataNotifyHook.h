// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE_ThreeParams(FPropertyActionShowChanged, const FPropertyChangedEvent&, FEditPropertyChain*, FSkillShowData*)

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API ActionShowDataNotifyHook : public FNotifyHook
{
public:
	ActionShowDataNotifyHook(UDataTable* InLoadedSkillShowDataTable, FPropertyActionShowChanged InOnPropertyChanged)
	{
		LoadedSkillShowDataTable = InLoadedSkillShowDataTable;
		OnPropertyChanged = InOnPropertyChanged;
	}
	virtual ~ActionShowDataNotifyHook() {}

	virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged) override
	{
		LoadedSkillShowDataTable->MarkPackageDirty();
		if (OnPropertyChanged.IsBound())
		{
			OnPropertyChanged.Execute(PropertyChangedEvent, PropertyThatChanged, CrrSkillShowData);
		}
	}

	UDataTable* LoadedSkillShowDataTable;
	FPropertyActionShowChanged OnPropertyChanged;
	FSkillShowData* CrrSkillShowData = nullptr;
};
