// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "IPropertyTypeCustomization.h"
#include "CoreMinimal.h"

/**
 * 
 */
class FShowCamSequenceKeyCustomization : public IPropertyTypeCustomization
{
public:
    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
    void CustomizePathPoints(IDetailChildrenBuilder& StructBuilder, TSharedPtr<IPropertyHandle> PathPointsHandle);
};