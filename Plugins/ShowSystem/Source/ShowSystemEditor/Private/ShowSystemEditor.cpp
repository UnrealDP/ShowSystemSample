#include "ShowSystemEditor.h"
#include "AssetTypeActions_ShowSequencer.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "ShowSequencerCustomization.h"


DEFINE_LOG_CATEGORY(ShowSystemEditor);

#define LOCTEXT_NAMESPACE "FShowSystemEditor"

void FShowSystemEditor::StartupModule()
{
	UE_LOG(ShowSystemEditor, Warning, TEXT("ShowSystemEditor module has been loaded"));

	// 에디터에서 Asset Tools 모듈 로드 -----------------------------------------------------------------------------------------------------------------------------
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// 새로운 카테고리 추가
	EAssetTypeCategories::Type ShowSystemAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("ShowSystem")), FText::FromString("Show System"));

	// ShowSequencer 액션 등록
	TSharedRef<IAssetTypeActions> ShowSequencerAction = MakeShareable(new FAssetTypeActions_ShowSequencer(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(ShowSequencerAction);
	RegisteredAssetTypeActions.Add(ShowSequencerAction);	


	// PropertyEditor 모듈을 로드하여 커스터마이저를 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	// UShowSequencer 클래스에 대한 커스터마이징 등록
	PropertyModule.RegisterCustomClassLayout("ShowSequencer",
		FOnGetDetailCustomizationInstance::CreateStatic(&FShowSequencerCustomization::MakeInstance));


	PropertyModule.NotifyCustomizationModuleChanged();
}

void FShowSystemEditor::ShutdownModule()
{
	// 모듈이 종료될 때 등록된 애셋 액션을 해제
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedPtr<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
		{
			if (Action.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
			}
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("ShowSequencer");
	}

	UE_LOG(ShowSystemEditor, Warning, TEXT("ShowSystemEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemEditor, ShowSystemEditor)