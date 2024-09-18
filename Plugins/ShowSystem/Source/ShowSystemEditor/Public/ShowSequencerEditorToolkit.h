// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Toolkits/AssetEditorToolkit.h"

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
    void GenerateExtendMenuBar();  // 메뉴바 생성 함수 추가

    class UShowSequencer* ShowSequencer;
};

