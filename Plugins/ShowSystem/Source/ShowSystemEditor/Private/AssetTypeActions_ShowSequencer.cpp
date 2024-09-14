// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_ShowSequencer.h"

FText FAssetTypeActions_ShowSequencer::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ShowSequencer", "Show Sequencer");
}

FColor FAssetTypeActions_ShowSequencer::GetTypeColor() const
{
	return FColor::Red;  // Customize asset color in the editor
}

UClass* FAssetTypeActions_ShowSequencer::GetSupportedClass() const
{
	return UShowSequencer::StaticClass();
}

uint32 FAssetTypeActions_ShowSequencer::GetCategories()
{
	return ShowSystemAssetCategory;  // Define the category in which this asset appears
}