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

	template<typename T, typename = std::enable_if_t<std::is_base_of<FShowKey, T>::value>>
	TObjectPtr<UShowBase> AddKey()
	{
		FInstancedStruct NewKey;
		NewKey.InitializeAs<T>();
		return AddKey(NewKey);
	}
	TObjectPtr<UShowBase> AddKey(FInstancedStruct& NewKey);
	bool RemoveKey(TObjectPtr<UShowBase> RemoveShowBase);

	UScriptStruct* GetShowKeyStaticStruct(UShowBase* ShowBase);
	FShowKey* GetMutableShowKey(UShowBase* ShowBase);

	void NotifyShowKeyChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged);

	void Play();

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

