// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStructureDetailsView.h"
#include "Widgets/SCompoundWidget.h"

struct FSkillData;
struct FSkillShowData;
class ActionDataNotifyHook;
class ActionShowDataNotifyHook;

DECLARE_DELEGATE_ThreeParams(FSelectActionFunction, FName, FSkillData*, FSkillShowData*)
DECLARE_DELEGATE_ThreeParams(FPropertyActionDataChanged, const FPropertyChangedEvent&, FEditPropertyChain*, FSkillData*)
DECLARE_DELEGATE_ThreeParams(FPropertyActionShowDataChanged, const FPropertyChangedEvent&, FEditPropertyChain*, FSkillShowData*)

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SSkillDataDetailsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSkillDataDetailsWidget) {}
		SLATE_ARGUMENT(UDataTable*, SkillDataTable)
		SLATE_ARGUMENT(UDataTable*, SkillShowDataTable)
		SLATE_EVENT(FSelectActionFunction, OnSelectAction)
		SLATE_EVENT(FPropertyActionDataChanged, OnActionPropertyChanged)
		SLATE_EVENT(FPropertyActionShowDataChanged, OnActionShowPropertyChanged)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> GenerateComboBoxItem(FName InItem);
	void OnSkillSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnSkillSelected(FString NewSelection);
	void OnTabChanged(int32 TabIndex);
	FName FirstSkillName() const { return SkillNames.Num() > 0 ? SkillNames[0] : NAME_None; }

	FSelectActionFunction OnSelectAction;
	UDataTable* SkillDataTable = nullptr;
	UDataTable* SkillShowDataTable = nullptr;

	TSharedPtr<IStructureDetailsView> SkillDetailsView = nullptr;
	TSharedPtr<IStructureDetailsView> SkillShowDetailsView = nullptr;
	TArray<FName> SkillNames;
	FName SelectedSkillName = NAME_None;
	int SelectedTabIndex = 0;
	TSharedPtr<SBox> DetailsContentBox;

	TSharedPtr<ActionDataNotifyHook> ActionDataNotifyHookInstance = nullptr;
	TSharedPtr<ActionShowDataNotifyHook> ActionShowDataNotifyHookInstance = nullptr;
	
};
