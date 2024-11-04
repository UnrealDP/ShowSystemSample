// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

struct FShowCamSequenceKey;
struct FCameraPathPoint;

DECLARE_DELEGATE_TwoParams(FOnButtonClicked, FShowCamSequenceKey*, FCameraPathPoint*);


/**
 * 
 */
class SHOWSYSTEMEDITOR_API FCameraPathPointCustom : public IPropertyTypeCustomization
{
public:

    static TSharedRef<IPropertyTypeCustomization> MakeInstance();

    virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
    virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

    FShowCamSequenceKey* ShowCamSequenceKey = nullptr;
    FCameraPathPoint* CameraPathPoint = nullptr;
};
