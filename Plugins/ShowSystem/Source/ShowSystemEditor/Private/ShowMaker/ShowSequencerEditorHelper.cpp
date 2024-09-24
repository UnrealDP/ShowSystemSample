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