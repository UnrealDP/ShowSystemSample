// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShowSystemEditorModeCommands.h"
#include "ShowSystemEditorMode.h"

#define LOCTEXT_NAMESPACE "ShowSystemEditorModeCommands"

FShowSystemEditorModeCommands::FShowSystemEditorModeCommands()
	: TCommands<FShowSystemEditorModeCommands>("ShowSystemEditorMode",
		NSLOCTEXT("ShowSystemEditorMode", "ShowSystemEditorModeCommands", "ShowSystem Editor Mode"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FShowSystemEditorModeCommands::RegisterCommands()
{
	TArray <TSharedPtr<FUICommandInfo>>& ToolCommands = Commands.FindOrAdd(NAME_Default);

	UI_COMMAND(SimpleTool, "Show Actor Info", "Opens message box with info about a clicked actor", EUserInterfaceActionType::Button, FInputChord());
	ToolCommands.Add(SimpleTool);

	UI_COMMAND(InteractiveTool, "Measure Distance", "Measures distance between 2 points (click to set origin, shift-click to set end point)", EUserInterfaceActionType::ToggleButton, FInputChord());
	ToolCommands.Add(InteractiveTool);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> FShowSystemEditorModeCommands::GetCommands()
{
	return FShowSystemEditorModeCommands::Get().Commands;
}

#undef LOCTEXT_NAMESPACE
