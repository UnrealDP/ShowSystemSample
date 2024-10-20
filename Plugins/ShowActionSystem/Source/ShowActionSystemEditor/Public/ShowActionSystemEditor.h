#pragma once

#include "Modules/ModuleManager.h"
#include "ActionBase.h"

DECLARE_LOG_CATEGORY_EXTERN(ShowActionSystemEditor, All, All);

class AShowActionMakerGameMode;
class UShowBase;
class SShowActionControllPanels;
class SSkillDataDetailsWidget;
class ShowSequencerNotifyHook;
class FShowSequencerEditorHelper;
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
	void UpdateShowKeyDetails(TSharedPtr<FShowSequencerEditorHelper> EditorHelper);

	void NotifyActionChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillData* SkillData);
	void NotifyActionShowChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillShowData* SkillShowData);
	void ChangeShow(EActionState ActionState, FSkillShowData* SkillShowData);

	FTabId SkillDataTabId;
	TObjectPtr<AShowActionMakerGameMode> ShowActionMakerGameMode = nullptr;

	TSharedPtr<SSkillDataDetailsWidget> SkillDataDetailsWidget = nullptr;

	FTabId ShowKeyDetailsTabId;
	TSharedPtr<ShowSequencerNotifyHook> ShowKeyNotifyHookInstance = nullptr;
	TSharedPtr<IStructureDetailsView> ShowKeyStructureDetailsView = nullptr;

	FTabId ShowActionControllPanelsTabId;
	TSharedPtr<SShowActionControllPanels> ShowActionControllPanels = nullptr;
	
};