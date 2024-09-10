// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShowSystemEditorMode.h"
#include "ShowSystemEditorModeToolkit.h"
#include "EdModeInteractiveToolsContext.h"
#include "InteractiveToolManager.h"
#include "ShowSystemEditorModeCommands.h"


//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
// AddYourTool Step 1 - include the header file for your Tools here
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////// 
#include "Tools/ShowSystemSimpleTool.h"
#include "Tools/ShowSystemInteractiveTool.h"

// step 2: register a ToolBuilder in FShowSystemEditorMode::Enter() below


#define LOCTEXT_NAMESPACE "ShowSystemEditorMode"

const FEditorModeID UShowSystemEditorMode::EM_ShowSystemEditorModeId = TEXT("EM_ShowSystemEditorMode");

FString UShowSystemEditorMode::SimpleToolName = TEXT("ShowSystem_ActorInfoTool");
FString UShowSystemEditorMode::InteractiveToolName = TEXT("ShowSystem_MeasureDistanceTool");


UShowSystemEditorMode::UShowSystemEditorMode()
{
	FModuleManager::Get().LoadModule("EditorStyle");

	// appearance and icon in the editing mode ribbon can be customized here
	Info = FEditorModeInfo(UShowSystemEditorMode::EM_ShowSystemEditorModeId,
		LOCTEXT("ModeName", "ShowSystem"),
		FSlateIcon(),
		true);
}


UShowSystemEditorMode::~UShowSystemEditorMode()
{
}


void UShowSystemEditorMode::ActorSelectionChangeNotify()
{
}

void UShowSystemEditorMode::Enter()
{
	UEdMode::Enter();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// AddYourTool Step 2 - register the ToolBuilders for your Tools here.
	// The string name you pass to the ToolManager is used to select/activate your ToolBuilder later.
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////// 
	const FShowSystemEditorModeCommands& SampleToolCommands = FShowSystemEditorModeCommands::Get();

	RegisterTool(SampleToolCommands.SimpleTool, SimpleToolName, NewObject<UShowSystemSimpleToolBuilder>(this));
	RegisterTool(SampleToolCommands.InteractiveTool, InteractiveToolName, NewObject<UShowSystemInteractiveToolBuilder>(this));

	// active tool type is not relevant here, we just set to default
	GetToolManager()->SelectActiveToolType(EToolSide::Left, SimpleToolName);
}

void UShowSystemEditorMode::CreateToolkit()
{
	Toolkit = MakeShareable(new FShowSystemEditorModeToolkit);
}

TMap<FName, TArray<TSharedPtr<FUICommandInfo>>> UShowSystemEditorMode::GetModeCommands() const
{
	return FShowSystemEditorModeCommands::Get().GetCommands();
}

#undef LOCTEXT_NAMESPACE
