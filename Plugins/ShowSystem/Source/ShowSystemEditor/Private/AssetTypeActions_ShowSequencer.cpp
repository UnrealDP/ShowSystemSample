// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_ShowSequencer.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowSystemEditor.h"

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
    if (InObjects.Num() == 1)  // 단일 객체만 처리
    {
        if (UShowSequencer* ShowSequencer = Cast<UShowSequencer>(InObjects[0]))
        {
            TSharedRef<FShowSequencerEditorToolkit> EditorToolkit(new FShowSequencerEditorToolkit());
            EditorToolkit->InitEditor(EToolkitMode::Standalone, EditWithinLevelEditor, ShowSequencer);

            /*FShowSystemEditor& ShowSystemEditorModule = FModuleManager::LoadModuleChecked<FShowSystemEditor>("ShowSystemEditor");
            ShowSystemEditorModule.OpenShowMakerTab(ShowSequencer);*/
        }
    }
    else
    {
        // 팝업을 통해 사용자에게 다중 선택에 대한 경고 메시지를 표시
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Only one object can be edited at a time. Please select a single object."));
    }
}