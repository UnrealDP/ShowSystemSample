// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"
#include "RunTime/Animation/AnimContainer.h"

/**
 * 
 */
class SHOWSYSTEMEDITOR_API FAssetTypeActions_AnimContainer : public FAssetTypeActions_Base
{
public:
	// 생성자에서 카테고리를 전달받도록 설정
	FAssetTypeActions_AnimContainer(EAssetTypeCategories::Type InAssetCategory)
		: AnimControllerAssetCategory(InAssetCategory)
	{
	}

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

	EAssetTypeCategories::Type AnimControllerAssetCategory;
};
