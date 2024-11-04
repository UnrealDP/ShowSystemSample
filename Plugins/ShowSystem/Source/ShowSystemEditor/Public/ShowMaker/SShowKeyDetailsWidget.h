// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStructureDetailsView.h"
#include "Widgets/SCompoundWidget.h"

class UShowBase;
class ShowSequencerNotifyHook;
class FShowSequencerEditorHelper;

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowKeyDetailsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowKeyDetailsWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, InEditorHelper)
		SLATE_ARGUMENT(UShowBase*, InShowBasePtr)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);	

	void UpdateEditorHelper(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper);
	void SetShowKey(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper, UShowBase* InShowBasePtr);

private:
	TSharedRef<SWidget> GetWidget();
	TSharedRef<SWidget> DefaultWidget();
	TSharedRef<SWidget> CamSequenceWidget();

private:
	TSharedPtr<ShowSequencerNotifyHook> ShowKeyNotifyHookInstance = nullptr;
	TSharedPtr<IStructureDetailsView> ShowKeyStructureDetailsView = nullptr;

	UShowBase* ShowBasePtr = nullptr;
	TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
};
