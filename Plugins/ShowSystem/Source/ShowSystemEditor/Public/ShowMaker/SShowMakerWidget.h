// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowSequencer.h"
#include "IPersonaPreviewScene.h"
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

	UWorld* CheckLoadWorld();
	USkeletalMesh* CheckLoadSkeletalMesh();	

	TSharedRef<SWidget> CreateMenuBar();
	TSharedRef<SWidget> GenerateMenuContent();
	FReply OnMenuButtonClicked();
	void GenerateMenu(FMenuBuilder& MenuBuilder);

	TSharedRef<SWidget> ConstructPreviewScenePanel(bool bDisplayAnimScrubBarEditing);

	void MakeFileMenu(FMenuBuilder& MenuBuilder);
	void OnOpenFile();

	//TSharedRef<class IPersonaToolkit> GetPersonaToolkit() const { return PersonaToolkit.ToSharedRef(); }

	/** Accessors to the current viewed Min/Max input range of the editor */
	float GetViewMinInput() const { return ViewMinInput; }
	float GetViewMaxInput() const { return ViewMaxInput; }
	void SetInputViewRange(float InViewMinInput, float InViewMaxInput);
	void HandleOnPreviewSceneSettingsCustomized(IDetailLayoutBuilder& DetailBuilder);
	void HandlePreviewSceneCreated(const TSharedRef<IPersonaPreviewScene>& InPreviewScene);

	void OpenSkeletalMeshPicker();
	void OnSkeletalMeshSelected(const FAssetData& SelectedAsset);

	void OpenWorldPicker();
	void OnWorldSelected(const FAssetData& SelectedAsset);

private:

	TSharedRef<SDockTab> SpawnTab_Preview();
	TSharedRef<SWidget> SpawnWidget_Preview();

	void RefreshPreviewViewport();
	bool SetPreviewAsset(UObject* InAsset);
	void UpdatePreviewViewportsVisibility();

	/** Preview Viewport widget */
	TSharedPtr<class SActorPreviewViewport> PreviewViewport;

	//TSharedPtr<class SViewport> ViewportWidget;  // 뷰포트 위젯
	//TSharedPtr<class FActorPreviewScene> PreviewScene;  // 미리보기 씬
	
	/** Persona toolkit */
	/*TSharedPtr<class IPersonaToolkit> PersonaToolkit;

	TWeakPtr<IPersonaPreviewScene> PreviewScenePtr;*/

	/** The editors Animation Scrub Panel */
	TSharedPtr<class SShowSequencerScrubPanel> ShowSequencerScrubPanel;

	TSharedPtr<SWindow> SkeletalMeshPickerWindow;
	TSharedPtr<SWindow> WorldPickerWindow;

	/** Get Min/Max Input of value **/
	float ViewMinInput;
	float ViewMaxInput;

	TObjectPtr<UShowSequencer> EditShowSequencer;
	TObjectPtr<UWorld> LoadedWorld;
	TObjectPtr<USkeletalMesh> LoadedSkeletalMesh;
};
