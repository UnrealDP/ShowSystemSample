#include "ShowSystemEditor.h"
#include "AssetTypeActions_ShowSequencer.h"
#include "AssetToolsModule.h"

DEFINE_LOG_CATEGORY(ShowSystemEditor);

#define LOCTEXT_NAMESPACE "FShowSystemEditor"

void FShowSystemEditor::StartupModule()
{
	UE_LOG(ShowSystemEditor, Warning, TEXT("ShowSystemEditor module has been loaded"));

	// 에디터에서 Asset Tools 모듈 로드
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// 새로운 카테고리 추가
	EAssetTypeCategories::Type ShowSystemAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("ShowSystem")), FText::FromString("Show System"));

	// ShowSequencer 액션 등록
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_ShowSequencer(ShowSystemAssetCategory)));
}

void FShowSystemEditor::ShutdownModule()
{
	UE_LOG(ShowSystemEditor, Warning, TEXT("ShowSystemEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemEditor, ShowSystemEditor)