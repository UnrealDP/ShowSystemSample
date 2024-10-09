#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ShowSystemEditor, All, All);

class FShowSystemEditor : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
	/*TSharedRef<SDockTab> OnSpawnShowMakerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SDockTab> OpenShowMakerTab(class UShowSequencer* ShowSequencer);
	void AddMenuExtension();*/
	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제

	/** 등록된 AssetTypeActions를 저장 */
	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;

	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
//private:
//	class UShowSequencer* TempShowSequencer;
	// -> FShowSystemEditor Module 의 전역 ShowMakerTab 기능은 삭제
};