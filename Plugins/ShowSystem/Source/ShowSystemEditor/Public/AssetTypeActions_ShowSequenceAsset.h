// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetTypeActions_Base.h"

class FAssetTypeActions_ShowSequenceAsset : public FAssetTypeActions_Base
{
public:
	// 생성자에서 카테고리를 전달받도록 설정
	FAssetTypeActions_ShowSequenceAsset(EAssetTypeCategories::Type InAssetCategory)
		: ShowSystemAssetCategory(InAssetCategory)
	{
	}

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor) override;

	EAssetTypeCategories::Type ShowSystemAssetCategory;
};