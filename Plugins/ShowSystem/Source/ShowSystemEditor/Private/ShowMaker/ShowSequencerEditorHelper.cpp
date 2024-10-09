// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "RunTime/ShowBase.h"

FShowSequencerEditorHelper::FShowSequencerEditorHelper()
{
}

FShowSequencerEditorHelper::~FShowSequencerEditorHelper()
{
	EditShowSequencer = nullptr;
}

void FShowSequencerEditorHelper::SetShowSequencerEditor(UShowSequencer* Sequencer)
{
	if (!Sequencer)
	{
		EditShowSequencer = nullptr;
		return;
	}

	if (EditShowSequencer)
	{
		EditShowSequencer->Dispose();
		EditShowSequencer = nullptr;
	}

	EditShowSequencer = Sequencer;
	EditShowSequencer->EditorInitialize();
	EditShowSequencer->SetDontDestroy();
}

void FShowSequencerEditorHelper::Play()
{
	if (EditShowSequencer)
	{
		EShowSequencerState showSequencerState = EditShowSequencer->GetShowSequencerState();
		switch (showSequencerState)
		{
		case EShowSequencerState::ShowSequencer_Wait:
			EditShowSequencer->EditorPlay();
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			EditShowSequencer->EditorUnPause();
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencer->EditorPlay();
			break;
		default:
			break;
		}
	}
}

TArray<FShowKey*> FShowSequencerEditorHelper::GetShowKeys()
{
	if (!EditShowSequencer)
	{
		return TArray<FShowKey*>();
	}

	TArray<FShowKey*> ShowKeyPointers;

	for (FInstancedStruct& Struct : EditShowSequencer->ShowKeys)
	{
		if (Struct.IsValid())
		{
			FShowKey* ShowKey = Struct.GetMutablePtr<FShowKey>();
			ShowKeyPointers.Add(ShowKey);
		}
	}

	return ShowKeyPointers;
}