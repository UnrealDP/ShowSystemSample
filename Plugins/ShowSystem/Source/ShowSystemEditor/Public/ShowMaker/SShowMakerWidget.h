// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "Widgets/SCompoundWidget.h"

class SActorPreviewViewport;
class SShowSequencerScrubPanel;
class IStructureDetailsView;
class FShowSequencerEditorHelper;
class UShowPlayer;
class ShowSequencerNotifyHook;

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
	
	virtual ~SShowMakerWidget()
	{
		if (EditShowSequencer)
		{
			EditShowSequencer->EditorBeginDestroy();
		}
	}

private:
	TSharedRef<SWidget> ConstructMainBody();
	TSharedRef<SWidget> ConstructPreviewScenePanel();
	TSharedRef<SWidget> ConstructShowKeyDetails();
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

	void SetShowKey(FShowKey* NewShowKey);

	/** Preview Viewport widget */
	TSharedPtr<SActorPreviewViewport> PreviewViewport = nullptr;
	TSharedPtr<IStructureDetailsView> StructureDetailsView = nullptr;
	TSharedPtr<SDockTab> ShowKeyDetailsTab = nullptr;

	TSharedPtr<SShowSequencerScrubPanel> ShowSequencerScrubPanel = nullptr;

	TSharedPtr<SWindow> SkeletalMeshPickerWindow = nullptr;
	TSharedPtr<SWindow> WorldPickerWindow = nullptr;

	TObjectPtr<UShowSequencer> EditShowSequencer = nullptr;
	TObjectPtr<UWorld> LoadedWorld = nullptr;
	TObjectPtr<USkeletalMesh> LoadedSkeletalMesh = nullptr;

	TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;

	AActor* Actor = nullptr;
	UShowPlayer* ShowPlayer = nullptr;
	FShowKey* SelectedShowKey = nullptr;
	TSharedPtr<ShowSequencerNotifyHook> NotifyHookInstance = nullptr;
};
