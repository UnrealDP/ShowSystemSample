// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowSequenceAssetFactory.h"
#include "RunTime/ShowSequenceAsset.h"

UShowSequenceAssetFactory::UShowSequenceAssetFactory()
{
	bCreateNew = true;  // Factory creates new objects
	bEditAfterNew = true;  // Open the object after creation
	SupportedClass = UShowSequenceAsset::StaticClass();
}

UObject* UShowSequenceAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Create a new instance of UShowSequenceAsset
	return NewObject<UShowSequenceAsset>(InParent, Class, Name, Flags);
}

bool UShowSequenceAssetFactory::ShouldShowInNewMenu() const
{
	// Show this factory in the "Create Advanced Asset" menu
	return true;
}
