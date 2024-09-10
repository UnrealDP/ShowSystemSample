// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShowSystemEditorModeToolkit.h"
#include "ShowSystemEditorMode.h"
#include "Engine/Selection.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "ShowSystemEditorModeToolkit"

FShowSystemEditorModeToolkit::FShowSystemEditorModeToolkit()
{
}

void FShowSystemEditorModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost, TWeakObjectPtr<UEdMode> InOwningMode)
{
	FModeToolkit::Init(InitToolkitHost, InOwningMode);
}

void FShowSystemEditorModeToolkit::GetToolPaletteNames(TArray<FName>& PaletteNames) const
{
	PaletteNames.Add(NAME_Default);
}


FName FShowSystemEditorModeToolkit::GetToolkitFName() const
{
	return FName("ShowSystemEditorMode");
}

FText FShowSystemEditorModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("DisplayName", "ShowSystemEditorMode Toolkit");
}

#undef LOCTEXT_NAMESPACE
