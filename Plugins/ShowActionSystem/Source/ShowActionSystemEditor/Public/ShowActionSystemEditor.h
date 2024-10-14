#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ShowActionSystemEditor, All, All);

class AShowActionMakerGameMode;
class UShowBase;
class SShowActionControllPanels;
struct FSkillData;
struct FSkillShowData;

class FShowActionSystemEditor : public IModuleInterface
{
public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	void RegisterSkillDataTab();
	void RegisterShowKeyDetailsTab();
	void RegisterShowActionControllPanelsTab();

	void RegisterMenus();
	void UnRegisterMenus();

	void OpenSkillDataDetails();
	void OpenShowKeyDetails();
	void OpenShowActionControllPanels();

	void SelectAction(FName InSelectedActionName, FSkillData* InSkillData, FSkillShowData* InSkillShowData);
	void SetShowBase(UShowBase* NewShowBase);

	FTabId SkillDataTabId;
	TObjectPtr<AShowActionMakerGameMode> ShowActionMakerGameMode = nullptr;

	FTabId SkillShowKeyDetailsTabId;
	TSharedPtr<IStructureDetailsView> StructureDetailsView = nullptr;
	UShowBase* SelectedShowBase = nullptr;

	FTabId ShowActionControllPanelsTabId;
	TSharedPtr<SShowActionControllPanels> ShowActionControllPanels = nullptr;
	
};