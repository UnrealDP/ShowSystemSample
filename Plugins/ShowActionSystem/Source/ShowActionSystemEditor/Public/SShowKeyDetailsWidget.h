// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStructureDetailsView.h"
#include "Widgets/SCompoundWidget.h"

struct FShowKey;

/**
 * 
 */
class SHOWACTIONSYSTEMEDITOR_API SShowKeyDetailsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowKeyDetailsWidget) {}
		SLATE_ARGUMENT(FShowKey*, ShowKey)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	void SetSkillKey(FShowKey* NewShowKey);

	TSharedPtr<IStructureDetailsView> StructureDetailsView = nullptr;
	FShowKey* SelectedShowKey = nullptr;
};
