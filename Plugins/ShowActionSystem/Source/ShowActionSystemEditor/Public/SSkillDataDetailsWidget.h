// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStructureDetailsView.h"
#include "Widgets/SCompoundWidget.h"

struct FSkillData;
DECLARE_DELEGATE_TwoParams(FOnComboBoxSelectionChanged, FName, FSkillData*)

typedef TFunction<void(FName, FSkillData*)> FSelectActionFunction;

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SSkillDataDetailsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSkillDataDetailsWidget) {}
		SLATE_ARGUMENT(UDataTable*, SkillDataTable)
		SLATE_ARGUMENT(FSelectActionFunction, OnSelectAction)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedRef<SWidget> GenerateComboBoxItem(FName InItem);
	void OnSkillSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	void OnTabChanged(int32 TabIndex);

	FSelectActionFunction OnSelectAction;
	UDataTable* SkillDataTable = nullptr;
	TSharedPtr<IStructureDetailsView> SkillDetailsView = nullptr;
	TSharedPtr<IStructureDetailsView> SkillShowDetailsView = nullptr;
	TArray<FName> SkillNames;
	FName SelectedSkillName = NAME_None;
	int SelectedTabIndex = 0;
	TSharedPtr<SBox> DetailsContentBox;
};
