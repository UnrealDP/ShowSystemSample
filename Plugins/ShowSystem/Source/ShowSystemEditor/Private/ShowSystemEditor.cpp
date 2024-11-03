#include "ShowSystemEditor.h"
#include "AssetTypeActions_ShowSequenceAsset.h"
#include "AssetToolsModule.h"
#include "PropertyEditorModule.h"
#include "ShowSequenceAssetCustomization.h"
#include "ShowMaker/SShowMakerWidget.h"
#include "Editor.h"
#include "AssetTypeActions_AnimContainer.h"
#include "AnimContainerFactory.h"
#include "ShowMaker/CameraPathPointCustom.h"
#include "RunTime/ShowKeys/ShowCamSequence.h"

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

	
	TSharedRef<IAssetTypeActions> AnimContainerAction = MakeShareable(new FAssetTypeActions_AnimContainer(ShowSystemAssetCategory));
	AssetTools.RegisterAssetTypeActions(AnimContainerAction);
	RegisteredAssetTypeActions.Add(AnimContainerAction);

	// Show Key 커스터마이징 디테일 창 ----
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout("ShowSequenceAsset",
		FOnGetDetailCustomizationInstance::CreateStatic(&FShowSequenceAssetCustomization::MakeInstance));
	
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"CameraPathPoint",
		FOnGetPropertyTypeCustomizationInstance::CreateLambda([this]() {

			FOnButtonClicked OnSelect = FOnButtonClicked::CreateRaw(this, &FShowSystemEditor::SelectCamkey);
			FOnButtonClicked OnSetCam = FOnButtonClicked::CreateRaw(this, &FShowSystemEditor::SetCamkey);

			TSharedRef<FCameraPathPointCustom> CustomizationInstance = MakeShareable(new FCameraPathPointCustom(OnSelect, OnSetCam));
			return CustomizationInstance;
			})
	);
	GetCameraLocationFunc = [this]() { return GetCameraLocation(); };
	// Show Key 커스터마이징 디테일 창 ----

	// ---  모듈 변경 알림  ---
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
		RegisteredAssetTypeActions.Empty();
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("ShowSequenceAsset");
		PropertyModule.UnregisterCustomPropertyTypeLayout("CameraPathPoint");
	}


	UE_LOG(ShowSystemEditor, Log, TEXT("ShowSystemEditor module has been unloaded"));
}


void FShowSystemEditor::SelectCamkey(FShowCamSequenceKey* ShowCamSequenceKey, FCameraPathPoint* CameraPathPoint)
{
	UE_LOG(LogTemp, Log, TEXT("SelectCamkey"));
}

void FShowSystemEditor::SetCamkey(FShowCamSequenceKey* ShowCamSequenceKey, FCameraPathPoint* CameraPathPoint)
{
	if (GetCameraLocationFunc)
	{
		FVector Location = GetCameraLocationFunc();
		UE_LOG(LogTemp, Log, TEXT("SetCamkey %s"), *Location.ToString());
	}
}

FVector FShowSystemEditor::GetCameraLocation()
{
	return FVector::ZeroVector;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemEditor, ShowSystemEditor)