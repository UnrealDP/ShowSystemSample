#pragma once

#include "Modules/ModuleManager.h"
#include <functional>

DECLARE_LOG_CATEGORY_EXTERN(ShowSystemEditor, All, All);

struct FShowCamSequenceKey;
struct FCameraPathPoint;

class FShowSystemEditor : public IModuleInterface
{
	public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	void SelectCamkey(FShowCamSequenceKey* ShowCamSequenceKey, FCameraPathPoint* CameraPathPoint);
	void SetCamkey(FShowCamSequenceKey* ShowCamSequenceKey, FCameraPathPoint* CameraPathPoint);

	/** 등록된 AssetTypeActions를 저장 */
	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;
};