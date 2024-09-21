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

	TSharedRef<SDockTab> OnSpawnShowMakerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SDockTab> OpenShowMakerTab(class UShowSequencer* ShowSequencer);
	void AddMenuExtension();

	/** 등록된 AssetTypeActions를 저장 */
	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;

private:
	class UShowSequencer* TempShowSequencer;
};