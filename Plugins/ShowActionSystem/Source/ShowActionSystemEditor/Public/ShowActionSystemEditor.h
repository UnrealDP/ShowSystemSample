#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ShowActionSystemEditor, All, All);

class AShowActionMakerGameMode;

class FShowActionSystemEditor : public IModuleInterface
{
public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	void RegisterMenus();
	void UnRegisterMenus();
	void OpenSkillDataDetails();
	void OpenSkillShowDataDetails();
	void OpenShowKeyDetails();

	FTabId SkillDataTabId;
	FTabId SkillShowDataTabId;
	TObjectPtr<AShowActionMakerGameMode> ShowActionMakerGameMode;
};