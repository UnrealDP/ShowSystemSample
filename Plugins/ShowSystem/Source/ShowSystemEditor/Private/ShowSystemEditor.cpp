#include "ShowSystemEditor.h"
#include "AssetTypeActions_ShowSequenceAsset.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "ShowSequenceAssetCustomization.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "Editor.h"
#include "AssetTypeActions_AnimContainer.h"
#include "AnimContainerFactory.h"


DEFINE_LOG_CATEGORY(ShowSystemEditor);

#define LOCTEXT_NAMESPACE "FShowSystemEditor"

void FShowSystemEditor::StartupModule()
{
	UE_LOG(ShowSystemEditor, Log, TEXT("ShowSystemEditor module has been loaded"));

	// 에디터에서 Asset Tools 모듈 로드 -----------------------------------------------------------------------------------------------------------------------------
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// 새로운 카테고리 추가
	EAssetTypeCategories::Type ShowSystemAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("ShowSystem")), FText::FromString("Show System"));

	// ShowSequencer 액션 등록
	TSharedRef<IAssetTypeActions> ShowSequencerAction = MakeShareable(new FAssetTypeActions_ShowSequenceAsset(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(ShowSequencerAction);
	RegisteredAssetTypeActions.Add(ShowSequencerAction);


	// PropertyEditor 모듈을 로드하여 커스터마이저를 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	// ShowSequenceAsset 클래스에 대한 커스터마이징 등록
	PropertyModule.RegisterCustomClassLayout("ShowSequenceAsset",
		FOnGetDetailCustomizationInstance::CreateStatic(&FShowSequenceAssetCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	
	TSharedRef<IAssetTypeActions> AnimContainerAction = MakeShareable(new FAssetTypeActions_AnimContainer(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(AnimContainerAction);
	RegisteredAssetTypeActions.Add(AnimContainerAction);
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
		RegisteredAssetTypeActions.Empty();
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("ShowSequenceAsset");
	}

	UE_LOG(ShowSystemEditor, Log, TEXT("ShowSystemEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemEditor, ShowSystemEditor)