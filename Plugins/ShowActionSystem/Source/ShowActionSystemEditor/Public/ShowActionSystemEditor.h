#pragma once

#include "Modules/ModuleManager.h"
#include "ActionBase.h"
#include "Misc/SortedPairArray.h"

DECLARE_LOG_CATEGORY_EXTERN(ShowActionSystemEditor, All, All);

class AShowActionMakerGameMode;
class UShowBase;
class SShowActionControllPanels;
class SSkillDataDetailsWidget;
class ShowSequencerNotifyHook;
class FShowSequencerEditorHelper;
struct FSkillData;
struct FSkillShowData;
struct FShowCamSequenceKey;
struct FCameraPathPoint;
class SShowKeyDetailsWidget;

class FShowActionSystemEditor : public IModuleInterface
{
public:

	/* Called when the module is loaded */
	virtual void StartupModule() override;

	/* Called when the module is unloaded */
	virtual void ShutdownModule() override;

	void InitializeModule(AShowActionMakerGameMode* InShowActionMakerGameMode);
	void ClearModule();

	void RegisterSkillDataTab();
	void RegisterShowKeyDetailsTab();
	void RegisterShowActionControllPanelsTab();

	void RegisterMenus();
	void UnRegisterMenus();

	void OpenSkillDataDetails();
	void OpenShowKeyDetails();
	void OpenShowActionControllPanels();

	void SelectAction(FName InSelectedActionName, FSkillData* InSkillData, FSkillShowData* InSkillShowData);
	void UpdateShowKeyDetails(TSharedPtr<FShowSequencerEditorHelper> InSelectedShowSequencerEditorHelper, UShowBase* InSelectedShowBasePtr);

	void NotifyActionChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillData* SkillData);
	void NotifyActionShowChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged, FSkillShowData* SkillShowData);
	void ChangeShow(EActionState ActionState, FSkillShowData* SkillShowData);
	void UpdateCameraPathPoint(APawn* InOwner, FCameraPathPoint* CameraPathPoint, const TArray<AActor*>& ActorsToHide);

private:
	// ----------------------------------------------------------------------------------------------
	// 편집 탭 윈도우 마지막 사용 도킹과 위치 등을 저장하게 하기 위해 StartupModule 에서 메인 탭에 등록
	FTabId SkillDataTabId;
	UPROPERTY()
	TWeakObjectPtr<AShowActionMakerGameMode> ShowActionMakerGameMode = nullptr;

	TSharedPtr<SSkillDataDetailsWidget> SkillDataDetailsWidget = nullptr;

	FTabId ShowKeyDetailsTabId;
	TSharedPtr<SShowKeyDetailsWidget> ShowKeyDetailsWidget = nullptr;	

	FTabId ShowActionControllPanelsTabId;
	TSharedPtr<SShowActionControllPanels> ShowActionControllPanels = nullptr;
	// ----------------------------------------------------------------------------------------------
	
	UActionBase* CrrActionPtr;
	TSortedPairArray<FString, TSharedPtr<FShowSequencerEditorHelper>> ShowSequencerEditorHelperSortMap;
	UShowBase* SelectedShowBasePtr = nullptr;

	float CrrTimeScale = 1.0f;
};