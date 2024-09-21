// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowMakerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowMakerWidget) {}
		SLATE_ARGUMENT(UShowSequencer*, EditShowSequencer)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> ConstructPreviewScenePanel();
	TSharedRef<SWidget> CreateMenuBar();
	TSharedRef<SWidget> GenerateMenuContent();
	FReply OnMenuButtonClicked();
	void GenerateMenu(FMenuBuilder& MenuBuilder);
	void MakeFileMenu(FMenuBuilder& MenuBuilder);
	void OnOpenFile();

	USkeletalMesh* CheckLoadSkeletalMesh();

	void RefreshPreviewViewport();
	bool SetPreviewAsset(UObject* InAsset);
	void UpdatePreviewViewportsVisibility();

	void OpenSkeletalMeshPicker();
	void OnSkeletalMeshSelected(const FAssetData& SelectedAsset);


	/** Preview Viewport widget */
	TSharedPtr<class SActorPreviewViewport> PreviewViewport;

	TSharedPtr<class SShowSequencerScrubPanel> ShowSequencerScrubPanel;

	TSharedPtr<SWindow> SkeletalMeshPickerWindow;
	TSharedPtr<SWindow> WorldPickerWindow;

	TObjectPtr<UShowSequencer> EditShowSequencer;
	TObjectPtr<UWorld> LoadedWorld;
	TObjectPtr<USkeletalMesh> LoadedSkeletalMesh;
};
