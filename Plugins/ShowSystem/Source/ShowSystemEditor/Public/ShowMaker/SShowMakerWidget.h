// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnShowKeyEvent, FShowKey*);

class SActorPreviewViewport;
class SShowSequencerScrubPanel;
class IStructureDetailsView;
class FShowSequencerEditorHelper;
class ShowSequencerNotifyHook;
class FShowSequencerEditorToolkit;

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowMakerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowMakerWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, EditorHelper)
		SLATE_ARGUMENT(UShowSequencer*, EditShowSequencer)
		SLATE_EVENT(FOnShowKeyEvent, OnAddKey)
		SLATE_EVENT(FOnShowKeyEvent, OnRemoveKey)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	UWorld* GetPreviewWorld() const;
	SActorPreviewViewport* GetPreviewViewportPtr() const { return PreviewViewport.Get(); }

private:
	void SetShowKey(FShowKey* NewShowKey);

private:
	TSharedRef<SWidget> ConstructMainBody();
	TSharedRef<SWidget> ConstructPreviewScenePanel();
	TSharedRef<SWidget> ConstructShowKeyDetails();

	/** Preview Viewport widget */
	TSharedPtr<SActorPreviewViewport> PreviewViewport = nullptr;
	TSharedPtr<IStructureDetailsView> StructureDetailsView = nullptr;

	FShowKey* SelectedShowKey = nullptr;
	TSharedPtr<ShowSequencerNotifyHook> NotifyHookInstance = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
	TAttribute<EShowSequencerState> ShowSequencerState = EShowSequencerState::ShowSequencer_Wait;

	FOnShowKeyEvent OnAddKey = nullptr;
	FOnShowKeyEvent OnRemoveKey = nullptr;
};
