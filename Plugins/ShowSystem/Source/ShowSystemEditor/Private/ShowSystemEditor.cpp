#include "ShowSystemEditor.h"
#include "AssetTypeActions_ShowSequencer.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "ShowSequencerCustomization.h"
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
	TSharedRef<IAssetTypeActions> ShowSequencerAction = MakeShareable(new FAssetTypeActions_ShowSequencer(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(ShowSequencerAction);
	RegisteredAssetTypeActions.Add(ShowSequencerAction);


	// PropertyEditor 모듈을 로드하여 커스터마이저를 등록
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	// UShowSequencer 클래스에 대한 커스터마이징 등록
	PropertyModule.RegisterCustomClassLayout("ShowSequencer",
		FOnGetDetailCustomizationInstance::CreateStatic(&FShowSequencerCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();

	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
	/*FGlobalTabmanager::Get()->RegisterNomadTabSpawner("ShowMakerTab", FOnSpawnTab::CreateRaw(this, &FShowSystemEditor::OnSpawnShowMakerTab))
		.SetDisplayName(FText::FromString("Show Maker"))
		.SetMenuType(ETabSpawnerMenuType::Enabled);*/
		// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제

	
	TSharedRef<IAssetTypeActions> AnimContainerAction = MakeShareable(new FAssetTypeActions_AnimContainer(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(AnimContainerAction);
	RegisteredAssetTypeActions.Add(AnimContainerAction);

	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
	//AddMenuExtension();
	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
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
		PropertyModule.UnregisterCustomClassLayout("ShowSequencer");
	}

	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
	//FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("ShowMakerTab");
	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제

	UE_LOG(ShowSystemEditor, Log, TEXT("ShowSystemEditor module has been unloaded"));
}

// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
//TSharedRef<SDockTab> FShowSystemEditor::OnSpawnShowMakerTab(const FSpawnTabArgs& SpawnTabArgs)
//{
//	// 탭 매니저를 가져옴
//	TSharedPtr<SWindow> OwnerWindow = SpawnTabArgs.GetOwnerWindow();
//
//	return SNew(SDockTab)
//		.TabRole(ETabRole::NomadTab)
//		[
//			SNew(SShowMakerWidget)
//				.EditShowSequencer(TempShowSequencer)
//		];
//}
//
//TSharedPtr<SDockTab> FShowSystemEditor::OpenShowMakerTab(UShowSequencer* ShowSequencer)
//{
//	TempShowSequencer = ShowSequencer;
//	return FGlobalTabmanager::Get()->TryInvokeTab(FName("ShowMakerTab"));
//}
//
//void FShowSystemEditor::AddMenuExtension()
//{
//	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu.Window");
//
//	// "WindowLayout" 섹션 추가
//	FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
//
//	// "ShowSystem" 이름이 붙은 섹션을 추가 (구분선 역할)
//	FToolMenuSection& ShowSystemSection = Menu->AddSection("ShowSystem", FText::FromString("ShowSystem"));
//
//	// "ShowSystem" 섹션 아래에 "ShowMaker" 메뉴 추가
//	ShowSystemSection.AddMenuEntry(
//		"ShowMaker",
//		FText::FromString("ShowMaker"),
//		FText::FromString("Open ShowMaker Window"),
//		FSlateIcon(),
//		FUIAction(FExecuteAction::CreateLambda([this]() {
//			// ShowMaker 창 여는 로직
//			OpenShowMakerTab(nullptr);
//			}))
//	);
//}
// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemEditor, ShowSystemEditor)