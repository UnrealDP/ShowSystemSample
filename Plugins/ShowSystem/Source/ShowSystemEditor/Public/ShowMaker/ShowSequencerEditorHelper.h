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
class SHOWSYSTEMEDITOR_API FShowSequencerEditorHelper : public FTickableEditorObject
{
public:
	FShowSequencerEditorHelper(TObjectPtr<UShowSequencer> InEditShowSequencer);
	~FShowSequencerEditorHelper();

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FShowSequencerEditorHelper, STATGROUP_Tickables);
	}
	virtual bool IsTickable() const override
	{
		return true;
	}

	void SetShowMakerWidget(TSharedPtr<SShowMakerWidget> InShowMakerWidget);
	TArray<TObjectPtr<UShowBase>>* RuntimeShowKeysPtr();
	void SetShowBaseStartTime(UShowBase* InShowBase, float StartTime);

	TObjectPtr<UShowBase> AddKey(FInstancedStruct& NewKey);
	bool RemoveKey(TObjectPtr<UShowBase> RemoveShowBase);
	int32 FindShowKeyIndex(const FShowKey* ShowKey) const;

	UScriptStruct* GetShowKeyStaticStruct(UShowBase* ShowBase);
	FShowKey* GetMutableShowKey(UShowBase* ShowBase);

	void NotifyShowKeyChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged);

	void Play();

	TObjectPtr<UShowBase> CheckGetSelectedShowBase();
	bool ValidateRuntimeShowKeys();
	bool ValidateShowAnimStatic(AActor* Owner, TObjectPtr<UShowBase>& ShowBase);

	UClass* GetLastSelectedActorClass();
	USkeletalMesh* LoadLastSelectedOrDefaultSkeletalMesh();	
	UClass* GetLastSelectedAnimInstanceClass();
	
	void ReplaceActorPreviewWorld(UClass* ActorClass);
	void ReplaceSkeletalMeshPreviewWorld(USkeletalMesh* SelectedSkeletalMesh);
	void ReplaceAnimInstancePreviewWorld(UClass* AnimInterfaceClass);

	TObjectPtr<UShowSequencer> EditShowSequencer = nullptr;
	TSharedPtr<SShowMakerWidget> ShowMakerWidget = nullptr;
	TObjectPtr<UShowBase> SelectedShowBase = nullptr;
};

