// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowMaker/ShowCamSequenceKeyCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "RunTime/ShowKeys//ShowCamSequence.h"


TSharedRef<IPropertyTypeCustomization> FShowCamSequenceKeyCustomization::MakeInstance()
{
    UE_LOG(LogTemp, Warning, TEXT("FShowCamSequenceKeyCustomization instance created"));
    return MakeShareable(new FShowCamSequenceKeyCustomization);
}

void FShowCamSequenceKeyCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    HeaderRow
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString("Edit Mode"))
                .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        .ValueContent()
        [
            SNew(SCheckBox)
                .OnCheckStateChanged_Lambda([](ECheckBoxState NewState)
                    {
                        bool bEditMode = (NewState == ECheckBoxState::Checked);
                        // 체크박스 상태에 따른 동작 정의
                    })
        ];
}

void FShowCamSequenceKeyCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
    TSharedPtr<IPropertyHandle> PathPointsHandle = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FShowCamSequenceKey, PathPoints));
    if (PathPointsHandle.IsValid() && PathPointsHandle->IsValidHandle())
    {
        CustomizePathPoints(StructBuilder, PathPointsHandle);
    }
}

void FShowCamSequenceKeyCustomization::CustomizePathPoints(IDetailChildrenBuilder& StructBuilder, TSharedPtr<IPropertyHandle> PathPointsHandle)
{
    uint32 NumElements;
    PathPointsHandle->GetNumChildren(NumElements);

    for (uint32 Index = 0; Index < NumElements; ++Index)
    {
        TSharedPtr<IPropertyHandle> ElementHandle = PathPointsHandle->GetChildHandle(Index);
        if (ElementHandle.IsValid() && ElementHandle->IsValidHandle())
        {
            FDetailWidgetRow& PathPointRow = StructBuilder.AddCustomRow(FText::FromString("Path Point"));

            PathPointRow
                .NameContent()
                [
                    ElementHandle->CreatePropertyNameWidget()
                ]
                .ValueContent()
                .MinDesiredWidth(200)
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                                .Text(FText::FromString("S Select"))
                                .OnClicked_Lambda([Index]()
                                    {
                                        // S Select 버튼 동작
                                        return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Set Cam"))
                                .OnClicked_Lambda([Index]()
                                    {
                                        // Set Cam 버튼 동작
                                        return FReply::Handled();
                                    })
                        ]
                ];
        }
    }
}