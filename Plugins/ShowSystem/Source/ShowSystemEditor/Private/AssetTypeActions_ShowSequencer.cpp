// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_ShowSequencer.h"
#include "ShowSequencerEditorToolkit.h"

FText FAssetTypeActions_ShowSequencer::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ShowSequencer", "Show Sequencer");
}

FColor FAssetTypeActions_ShowSequencer::GetTypeColor() const
{
	return FColor::Red;  // Customize asset color in the editor
}

UClass* FAssetTypeActions_ShowSequencer::GetSupportedClass() const
{
	return UShowSequencer::StaticClass();
}

uint32 FAssetTypeActions_ShowSequencer::GetCategories()
{
	return ShowSystemAssetCategory;  // Define the category in which this asset appears
}

// 애셋 더블클릭 시 커스텀 에디터 열기
void FAssetTypeActions_ShowSequencer::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    // ShowSequencer 에디터 툴킷 생성
    TSharedRef<FShowSequencerEditorToolkit> EditorToolkit = MakeShareable(new FShowSequencerEditorToolkit());

    // 에디터 초기화
    EditorToolkit->InitEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}