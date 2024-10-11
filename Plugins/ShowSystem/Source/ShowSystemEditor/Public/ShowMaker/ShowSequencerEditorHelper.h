// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"

class FShowSequencerEditorToolkit;
class SShowMakerWidget;

/**
 * 
 */
class SHOWSYSTEMEDITOR_API FShowSequencerEditorHelper
{
public:
	FShowSequencerEditorHelper(FShowSequencerEditorToolkit* InShowSequencerEditorToolkit, TObjectPtr<UShowSequencer> InEditShowSequencer);
	~FShowSequencerEditorHelper();

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
	void ShowSequencerDetailsViewForceRefresh();
	void SetShowMakerWidget(TSharedPtr<SShowMakerWidget> InShowMakerWidget);

	UClass* GetLastSelectedActorClass();
	USkeletalMesh* LoadLastSelectedOrDefaultSkeletalMesh();	
	UClass* GetLastSelectedAnimInstanceClass();
	
	void ReplaceActorPreviewWorld(UClass* ActorClass);
	void ReplaceSkeletalMeshPreviewWorld(USkeletalMesh* SelectedSkeletalMesh);
	void ReplaceAnimInstancePreviewWorld(UClass* AnimInterfaceClass);

	FShowSequencerEditorToolkit* ShowSequencerEditorToolkit = nullptr;
	TObjectPtr<UShowSequencer> EditShowSequencer = nullptr;
	TSharedPtr<SShowMakerWidget> ShowMakerWidget = nullptr;
	FShowKey* SelectedShowKey = nullptr;
};

