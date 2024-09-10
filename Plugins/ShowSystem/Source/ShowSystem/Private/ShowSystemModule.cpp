// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShowSystemModule.h"
#include "ShowSystemEditorModeCommands.h"

#define LOCTEXT_NAMESPACE "ShowSystemModule"

void FShowSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FShowSystemEditorModeCommands::Register();
}

void FShowSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FShowSystemEditorModeCommands::Unregister();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShowSystemModule, ShowSystemEditorMode)