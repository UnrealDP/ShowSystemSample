// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowSequencerCustomization.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowBase.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "SStructComboBoxWidget.h"
#include "IDetailGroup.h"
#include "RunTime/ShowKeys/ShowAnimStatic.h"

TSharedRef<IDetailCustomization> FShowSequencerCustomization::MakeInstance()
{
    return MakeShareable(new FShowSequencerCustomization);
}

//void FShowSequencerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
//{
//    CachedDetailBuilder = &DetailBuilder;
//
//    // 카테고리 및 InstancedStruct 속성 핸들 정의
//    IDetailCategoryBuilder& ShowCategory = DetailBuilder.EditCategory("Show");
//    TSharedRef<IPropertyHandle> ShowKeysHandle = DetailBuilder.GetProperty("ShowKeys", UShowSequencer::StaticClass());
//
//    // ShowKeys 배열 핸들로 변환
//    ShowKeysArrayHandle = ShowKeysHandle->AsArray();
//    if (!ShowKeysArrayHandle.IsValid())
//    {
//        // 배열로 변환되지 않았다면, 배열이 아님
//        UE_LOG(LogTemp, Error, TEXT("ShowKeys is not a valid array."));
//        return;
//    }
//
//    // 배열의 항목들을 편집 가능하게 표시
//    uint32 NumElements = 0;
//    ShowKeysArrayHandle->GetNumElements(NumElements);
//
//    for (uint32 Index = 0; Index < NumElements; ++Index)
//    {
//        TSharedRef<IPropertyHandle> ArrayElement = ShowKeysArrayHandle->GetElement(Index);
//
//        // Create a custom row for each array element
//        ShowCategory.AddCustomRow(FText::FromString(FString::Printf(TEXT("Key [%d]"), Index)))
//            .NameContent()
//            [
//                ArrayElement->CreatePropertyNameWidget()  // Display the name of the property
//            ]
//            .ValueContent()
//            [
//                SNew(STextBlock).Text(FText::FromString("Show Keys"))
//                //SNew(SStructComboBoxWidget)
//                //    //.ArrayElement(ArrayElement)
//                //    .BaseStruct(FShowKey::StaticStruct())
//                //    .bIncludeBaseStruct(false)
//                //    .OnSelectionChanged(this, &FShowSequencerCustomization::OnStructSelected_New)
//            ];
//    }
//}
//
//void FShowSequencerCustomization::OnStructSelected_New(TSharedPtr<IPropertyHandle> ArrayElement, UScriptStruct* NewSelection)
//{
//    if (NewSelection)
//    {
//        // Set the value of the selected struct inside the FInstancedStruct
//        FInstancedStruct* StructData = nullptr;
//        ArrayElement->GetValueData(reinterpret_cast<void*&>(StructData));
//
//        if (StructData)
//        {
//            StructData->InitializeAs(NewSelection);  // Set the struct type in the FInstancedStruct
//        }
//    }
//}

void FShowSequencerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    CachedDetailBuilder = &DetailBuilder;

    // UShowSequencer 클래스에서 ShowKeys 프로퍼티를 찾습니다.
    IDetailCategoryBuilder& ShowCategory = DetailBuilder.EditCategory("Show");

    // ShowKeys 배열 UI에 대한 핸들 생성
    TSharedRef<IPropertyHandle> ShowKeysHandleRef = DetailBuilder.GetProperty("ShowKeys", UShowSequencer::StaticClass());
    DetailBuilder.HideProperty(ShowKeysHandleRef);  // 기본 UI 숨김 처리

    // ShowKeys 배열 핸들로 변환
    ShowKeysArrayHandle = ShowKeysHandleRef->AsArray();
    if (!ShowKeysArrayHandle.IsValid())
    {
        // 배열로 변환되지 않았다면, 배열이 아님
        UE_LOG(LogTemp, Error, TEXT("ShowKeys is not a valid array."));
        return;
    }

    // ShowKeys 그룹을 만들어 ShowKeys 배열을 관리
    IDetailGroup& ShowKeysGroup = ShowCategory.AddGroup("ShowKeysGroup", FText::FromString("Show Keys"), false, true);

    ShowKeysGroup.HeaderRow()
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString("Show Keys"))
        ]
        .ValueContent()
        [
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.6f)
                [
                    SNew(SStructComboBoxWidget)
                        .BaseStruct(FShowKey::StaticStruct())
                        .bIncludeBaseStruct(false)
                        .OnSelectionChanged(this, &FShowSequencerCustomization::OnStructSelected)
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.2f)
                .VAlign(VAlign_Center)
                .AutoWidth()
                [
                    SNew(SButton)
                        .ButtonStyle(FAppStyle::Get(), "FlatButton")
                        .ContentPadding(2.0f)
                        .OnClicked(this, &FShowSequencerCustomization::OnAddButtonClicked)
                        [
                            SNew(SImage)
                                .Image(FAppStyle::GetBrush("PListEditor.Button_AddToArray"))
                        ]
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.2f)
                .VAlign(VAlign_Center)
                .AutoWidth()
                [
                    SNew(SButton)
                        .ButtonStyle(FAppStyle::Get(), "FlatButton")
                        .ContentPadding(2.0f)
                        .OnClicked(this, &FShowSequencerCustomization::OnEmptyButtonClicked)
                        [
                            SNew(SImage)
                                .Image(FAppStyle::GetBrush("ContentReference.Clear"))
                        ]
                ]
        ];

    // 배열의 항목들을 편집 가능하게 표시
    uint32 NumElements = 0;
    ShowKeysArrayHandle->GetNumElements(NumElements);

    for (uint32 Index = 0; Index < NumElements; ++Index)
    {
        TSharedRef<IPropertyHandle> ArrayElement = ShowKeysArrayHandle->GetElement(Index);

        FProperty* Property = ArrayElement->GetProperty();

        // Check if the property is a struct
        if (FStructProperty* StructProp = Cast<FStructProperty>(Property))
        {
            // Get the actual struct type
            UStruct* StructType = StructProp->Struct;

            if (StructType)
            {
                FString StructName = StructType->GetName();
                UE_LOG(LogTemp, Log, TEXT("Array Element Struct Type: %s"), *StructName);
            }

            void* StructData = nullptr;
            ArrayElement->GetValueData(StructData);

            if (StructData)
            {
                const FShowKey* ShowKey = reinterpret_cast<const FShowKey*>(StructData);
                FString StructName = ShowKey->StaticStruct()->GetFName().ToString();
                UE_LOG(LogTemp, Log, TEXT("----------------- Struct Type: %s"), *StructName);
            }
        }

        FString StructName = TEXT("ERR");
        if (Property && Property->IsA(FStructProperty::StaticClass()))
        {
            FStructProperty* StructProperty = CastField<FStructProperty>(Property);
            if (StructProperty)
            {
                StructName = StructProperty->Struct->GetFName().ToString();
            }
        }

        // ShowKeys 그룹에 배열 항목 추가
        ShowKeysGroup.AddPropertyRow(ArrayElement)
            .DisplayName(FText::FromString(FString::Printf(TEXT("%s [%d]"), *StructName, Index)));
    }
}


void FShowSequencerCustomization::OnStructSelected(UScriptStruct* NewSelection)
{
    if (NewSelection)
    {
        // 선택된 Struct 저장 (클래스 멤버 변수 접근)
        SelectedStruct = NewSelection;
        UE_LOG(LogTemp, Log, TEXT("선택된 Struct: %s"), *NewSelection->GetName());
    }
}

FReply FShowSequencerCustomization::OnAddButtonClicked()
{
    if (SelectedStruct)
    {
        // ShowKeys 배열에 새 항목 추가
        ShowKeysArrayHandle->AddItem();

        // 마지막 추가된 항목에 선택한 구조체 설정
        uint32 ArraySize = 0;
        ShowKeysArrayHandle->GetNumElements(ArraySize); 
        TSharedRef<IPropertyHandle> LastElement = ShowKeysArrayHandle->GetElement(ArraySize - 1);

        // 구조체 인스턴스 생성
        void* StructData = nullptr;
        LastElement->GetValueData(StructData);

        if (StructData)
        {
            // FInstancedStruct의 참조를 가져옴
            FInstancedStruct* InstancedStructPtr = static_cast<FInstancedStruct*>(StructData);
            if (InstancedStructPtr)
            {
                // FShowAnimKey로 초기화
                InstancedStructPtr->InitializeAs(SelectedStruct);
            }

            UE_LOG(LogTemp, Log, TEXT("Struct: %s가 배열에 추가됨"), *SelectedStruct->GetName());

            // Force a refresh of the details panel to immediately show the updated array
            if (CachedDetailBuilder)
            {
                CachedDetailBuilder->ForceRefreshDetails();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("추가할 Struct가 선택되지 않음"));
    }

    return FReply::Handled();
}

FReply FShowSequencerCustomization::OnEmptyButtonClicked()
{
    ShowKeysArrayHandle->EmptyArray();

    // Force a refresh of the details panel to immediately show the updated array
    if (CachedDetailBuilder)
    {
        CachedDetailBuilder->ForceRefreshDetails();
    }

    return FReply::Handled();
}