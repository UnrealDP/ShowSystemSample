// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Toolkits/AssetEditorToolkit.h"

class UShowSequencer;
class FShowSequencerEditorHelper;

class FShowSequencerEditorToolkit : public FAssetEditorToolkit
{
public:

    // FAssetEditorToolkit 인터페이스 구현
    virtual FName GetToolkitFName() const override { return FName("ShowSequencerEditor"); }
    virtual FText GetBaseToolkitName() const override { return FText::FromString("Show Sequencer Editor"); }
    virtual FString GetWorldCentricTabPrefix() const override { return FString("ShowSequencerEditor"); }
    virtual FLinearColor GetWorldCentricTabColorScale() const override { return FLinearColor::White; }

    virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

    void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, class UShowSequencer* ShowSequencer);

    // 기본 Details 탭 생성
    TSharedRef<SDockTab> SpawnDetailsTab(const FSpawnTabArgs& Args);

    // ShowMaker 탭 생성
    TSharedRef<SDockTab> SpawnShowMakerTab(const FSpawnTabArgs& Args);

protected:
    void GenerateToolbarButtons();
    void GenerateExtendMenuBar();
    void AddToolbarButtons(FToolBarBuilder& ToolbarBuilder);

    void OpenActorPicker();
    void OnSelectedActor(const FAssetData& SelectedAsset);

    void OpenSkeletalMeshPicker();
    void OnSelectedSkeletalMesh(const FAssetData& SelectedAsset);

    void OpenAnimPicker();
    void OnSelectedAnim(const FAssetData& SelectedAsset);

public:
    TSharedPtr<IDetailsView> DetailsView = nullptr;
    TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
};

