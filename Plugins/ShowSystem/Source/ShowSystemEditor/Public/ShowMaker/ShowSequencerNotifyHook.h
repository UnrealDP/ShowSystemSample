// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShowSequencerEditorHelper.h"
#include "Misc/NotifyHook.h"

/**
 * 
 */
class SHOWSYSTEMEDITOR_API ShowSequencerNotifyHook : public FNotifyHook
{
public:
    ShowSequencerNotifyHook(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper)
    {
        EditorHelper = InEditorHelper;
    }

    virtual ~ShowSequencerNotifyHook()
    {
        if (EditorHelper)
		{
            EditorHelper = nullptr;
		}
    }

    virtual void NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent, FEditPropertyChain* PropertyThatChanged) override
    {
        if (EditorHelper)
        {
            EditorHelper->NotifyShowKeyChange(PropertyChangedEvent, PropertyThatChanged);
        }
    }

    void UpdateEditorHelper(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper)
    {
        EditorHelper = InEditorHelper;
    }

private:
    TSharedPtr<FShowSequencerEditorHelper> EditorHelper = nullptr;
};
