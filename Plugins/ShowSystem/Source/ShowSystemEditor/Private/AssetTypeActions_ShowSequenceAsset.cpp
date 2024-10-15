// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_ShowSequenceAsset.h"
#include "RunTime/ShowSequenceAsset.h"
#include "ShowSequencerEditorToolkit.h"
#include "ShowSystemEditor.h"

FText FAssetTypeActions_ShowSequenceAsset::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "FAssetTypeActions_ShowSequenceAsset", "ShowSequence Asset");
}
 
FColor FAssetTypeActions_ShowSequenceAsset::GetTypeColor() const
{
	return FColor::Red;  // Customize asset color in the editor
}

UClass* FAssetTypeActions_ShowSequenceAsset::GetSupportedClass() const
{
	return UShowSequenceAsset::StaticClass();
}

uint32 FAssetTypeActions_ShowSequenceAsset::GetCategories()
{
	return ShowSystemAssetCategory;  // Define the category in which this asset appears
}

// 애셋 더블클릭 시 커스텀 에디터 열기
void FAssetTypeActions_ShowSequenceAsset::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    if (InObjects.Num() == 1)  // 단일 객체만 처리
    {
        if (UShowSequenceAsset* ShowSequenceAsset = Cast<UShowSequenceAsset>(InObjects[0]))
        {
            TSharedRef<FShowSequencerEditorToolkit> EditorToolkit(new FShowSequencerEditorToolkit());
            EditorToolkit->InitEditor(EToolkitMode::Standalone, EditWithinLevelEditor, ShowSequenceAsset);
        }
    }
    else
    {
        // 팝업을 통해 사용자에게 다중 선택에 대한 경고 메시지를 표시
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Only one object can be edited at a time. Please select a single object."));
    }
}