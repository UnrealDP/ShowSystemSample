// Fill out your copyright notice in the Description page of Project Settings.

#include "ShowMaker/CameraPathPointCustom.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "RunTime/ShowKeys/ShowCamSequence.h"

TSharedRef<IPropertyTypeCustomization> FCameraPathPointCustom::MakeInstance()
{
    return MakeShareable(new FCameraPathPointCustom());
}

void FCameraPathPointCustom::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    HeaderRow.NameContent()
        [
            StructPropertyHandle->CreatePropertyNameWidget()
        ];
}

void FCameraPathPointCustom::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    // 기본 프로퍼티 렌더링
    uint32 NumChildren;
    StructPropertyHandle->GetNumChildren(NumChildren);

    // 각 자식 프로퍼티 순회
    for (uint32 Index = 0; Index < NumChildren; ++Index)
    {
        TSharedPtr<IPropertyHandle> ChildProperty = StructPropertyHandle->GetChildHandle(Index); // 자식 프로퍼티 가져오기

        if (ChildProperty.IsValid())
        {
            // 기본 UI로 렌더링
            StructBuilder.AddProperty(ChildProperty.ToSharedRef());
        }
    }

    TArray<TSharedPtr<FStructOnScope>> Structs;
    StructPropertyHandle->GetOuterStructs(Structs);
    for (TSharedPtr<FStructOnScope> StructOnScope : Structs)
    {
        if (!StructOnScope.IsValid())
        {
            continue;
        }
            
        const UStruct* StructType = StructOnScope->GetStruct();
        if (!StructType)
        {
			continue;
		}

        if (StructType == FShowCamSequenceKey::StaticStruct())
		{
            uint8* StructMemory = StructOnScope->GetStructMemory();
            if (!StructMemory)
            {
                continue;
            }

            ShowCamSequenceKey = reinterpret_cast<FShowCamSequenceKey*>(StructMemory);
            break;
		}
    }

    FProperty* Property = StructPropertyHandle->GetProperty();
    if (Property)
    {
        if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
        {
            if (const UScriptStruct* StructType = StructProperty->Struct)
            {
                if (StructType == FCameraPathPoint::StaticStruct())
                {
                    void* StructData = nullptr;
                    if (StructPropertyHandle->GetValueData(StructData) == FPropertyAccess::Success && StructData)
                    {
                        CameraPathPoint = reinterpret_cast<FCameraPathPoint*>(StructData);
                    }
                }
            }
        }
    }

    // 버튼 2개 최상단에 추가
    StructBuilder.AddCustomRow(FText::FromString("Buttons"))
        .WholeRowContent()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(2.0f, 7.0f))
                [
                    SNew(SButton)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked_Lambda([this]()
                            {
                                for (FCameraPathPoint& cpp : ShowCamSequenceKey->PathPoints)
                                {
                                    cpp.bIsSelected = &cpp == CameraPathPoint;
                                }
                                return FReply::Handled();
                            })
                        [
                            SNew(STextBlock)
                                .Text(FText::FromString("Select"))
                                .ColorAndOpacity_Lambda([this]()
                                    {
                                        return CameraPathPoint->bIsSelected ? FLinearColor::Yellow : FLinearColor::White;
                                    })
                        ]
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(FMargin(2.0f, 7.0f))
                [
                    SNew(SButton)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .Text(FText::FromString("Set Cam"))
                        .OnClicked_Lambda([this]()
                            {
                                for (FCameraPathPoint& cpp : ShowCamSequenceKey->PathPoints)
                                {
                                    if (&cpp == CameraPathPoint)
                                    {
                                        cpp.bNeedUpdateLocation = true;
                                        break;
                                    }
                                }
                                return FReply::Handled();
                            })
                ]
        ];
}
