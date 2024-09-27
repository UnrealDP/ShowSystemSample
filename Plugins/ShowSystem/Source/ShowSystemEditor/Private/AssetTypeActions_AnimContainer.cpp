// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetTypeActions_AnimContainer.h"
#include "AnimContainerFactory.h"


FText FAssetTypeActions_AnimContainer::GetName() const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_AnimContainer", "Anim Container");
}

FColor FAssetTypeActions_AnimContainer::GetTypeColor() const
{
    return FColor::Red;  // Customize asset color in the editor
}

UClass* FAssetTypeActions_AnimContainer::GetSupportedClass() const
{
    return UAnimContainer::StaticClass();
}

uint32 FAssetTypeActions_AnimContainer::GetCategories()
{
    return AnimControllerAssetCategory;  // Define the category in which this asset appears
}
