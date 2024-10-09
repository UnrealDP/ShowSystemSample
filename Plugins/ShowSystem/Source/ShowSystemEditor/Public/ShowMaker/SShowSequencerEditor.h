// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RunTime/ShowBase.h"
#include "Widgets/SCompoundWidget.h"

class FShowSequencerEditorHelper;

DECLARE_DELEGATE_OneParam(FOnShowKeyEvent, FShowKey*);

/**
 * 
 */
class SHOWSYSTEMEDITOR_API SShowSequencerEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShowSequencerEditor) {}
		SLATE_ARGUMENT(TSharedPtr<FShowSequencerEditorHelper>, EditorHelper)
		SLATE_ATTRIBUTE(float, Height)
		SLATE_ATTRIBUTE(float, MinWidth)
		SLATE_ATTRIBUTE(float, SecondToWidthRatio)
		SLATE_EVENT(FOnShowKeyEvent, OnAddKey)
		SLATE_EVENT(FOnShowKeyEvent, OnRemoveKey)
		SLATE_EVENT(FOnShowKeyEvent, OnClickedKey)
		SLATE_EVENT(FOnShowKeyEvent, OnChangedKey)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
};
