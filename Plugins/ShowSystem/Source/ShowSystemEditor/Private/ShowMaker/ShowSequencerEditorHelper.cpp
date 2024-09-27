// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowSequencerEditorHelper.h"

FShowSequencerEditorHelper::FShowSequencerEditorHelper()
{
}

FShowSequencerEditorHelper::~FShowSequencerEditorHelper()
{
	EditShowSequencer = nullptr;
}

void FShowSequencerEditorHelper::SetShowSequencerEditor(UShowSequencer* Sequencer)
{
	checkf(Sequencer, TEXT("FShowSequencerEditorHelper::SetShowSequencerEditor: Sequencer is invalid."));

	if (EditShowSequencer)
	{
		EditShowSequencer->Dispose();
		EditShowSequencer = nullptr;
	}

	EditShowSequencer = Sequencer;
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
			EditShowSequencer->Play();
			break;
		case EShowSequencerState::ShowSequencer_Playing:
			break;
		case EShowSequencerState::ShowSequencer_Pause:
			EditShowSequencer->UnPause();
			break;
		case EShowSequencerState::ShowSequencer_End:
			EditShowSequencer->Play();
			break;
		default:
			break;
		}
	}
}