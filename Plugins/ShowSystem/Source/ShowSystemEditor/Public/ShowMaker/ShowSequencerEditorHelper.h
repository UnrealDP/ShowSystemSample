// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"

/**
 * 
 */
class SHOWSYSTEMEDITOR_API FShowSequencerEditorHelper
{
public:
	FShowSequencerEditorHelper();
	~FShowSequencerEditorHelper();

	void SetShowSequencerEditor(UShowSequencer* Sequencer);

	TArray<FShowKey*> GetShowKeys();

	template<typename T, typename = std::enable_if_t<std::is_base_of<FShowKey, T>::value>>
	FShowKey* AddKey()
	{
		FInstancedStruct NewKey;
		NewKey.InitializeAs<T>();
		FShowKey* NewShowKey = EditShowSequencer->EditorAddKey(NewKey);
		return NewShowKey;
	}

	void Play();

	TObjectPtr<UShowSequencer> EditShowSequencer;
};
