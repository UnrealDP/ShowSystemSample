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

	/** 등록된 AssetTypeActions를 저장 */
	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;
};