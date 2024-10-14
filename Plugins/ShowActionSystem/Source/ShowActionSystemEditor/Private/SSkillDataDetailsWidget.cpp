// Fill out your copyright notice in the Description page of Project Settings.


#include "SSkillDataDetailsWidget.h"
#include "SlateOptMacros.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
#include "SStringComboBoxWidget.h"
#include "DataTableManager.h"
#include "STabButtonWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSkillDataDetailsWidget::Construct(const FArguments& InArgs)
{
    SkillDataTable = InArgs._SkillDataTable;
    OnSelectAction = InArgs._OnSelectAction;

    // 스킬 테이블의 Key 값을 로드
    TArray<FString> SkillNameStrings;
    if (SkillDataTable)
    {
        for (const FName& RowName : SkillDataTable->GetRowNames())
        {
            SkillNames.Add(RowName);
            SkillNameStrings.Add(RowName.ToString());
        }
    }

    // SStructureDetailsView 설정
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs SkillDetailsViewArgs;
    SkillDetailsViewArgs.bAllowSearch = true;
    SkillDetailsViewArgs.bShowOptions = true;
    FStructureDetailsViewArgs SkillDetailsArgs;
    SkillDetailsView = PropertyEditorModule.CreateStructureDetailView(
        SkillDetailsViewArgs,
        SkillDetailsArgs,
        nullptr,
        FText::FromString("Skill Details")
    );

    FDetailsViewArgs SkillShowDetailsViewArgs;
    SkillShowDetailsViewArgs.bAllowSearch = true;
    SkillShowDetailsViewArgs.bShowOptions = true;
    FStructureDetailsViewArgs SkillShowDetailsArgs;    
    SkillShowDetailsView = PropertyEditorModule.CreateStructureDetailView(
        SkillShowDetailsViewArgs,
        SkillShowDetailsArgs,
        nullptr,
        FText::FromString("Skill Show Details")
    );

    // 레이아웃 생성
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SStringComboBoxWidget)
                        .Options(SkillNameStrings)
                        .OnSelectionChanged(this, &SSkillDataDetailsWidget::OnSkillSelected)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STabButtonWidget)
                        .TabNames({ "Skill Details", "Skill Show Details" })  // 탭 이름
                        .OnTabChanged(this, &SSkillDataDetailsWidget::OnTabChanged)  // 탭 변경 이벤트
                ]

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SAssignNew(DetailsContentBox, SBox)
                        [
                            SelectedTabIndex == 0 ?
                                SkillDetailsView->GetWidget().ToSharedRef() :
                                SkillShowDetailsView->GetWidget().ToSharedRef()
                        ]
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SSkillDataDetailsWidget::GenerateComboBoxItem(FName InItem)
{
    return SNew(STextBlock).Text(FText::FromName(InItem));
}

void SSkillDataDetailsWidget::OnSkillSelected(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    FName NewSelectionName = FName(*NewSelection);

    if (SelectedSkillName.IsEqual(NewSelectionName))
    {
        return;
    }

    if (SkillDataTable)
    {
        static const FString ContextString(TEXT("Skill Context"));
        FSkillData* SkillData = SkillDataTable->FindRow<FSkillData>(NewSelectionName, ContextString);

        if (SkillData)
        {
            if (SkillDetailsView.IsValid())
            {
                // 구조체 데이터
                TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(FSkillData::StaticStruct(), (uint8*)SkillData));
                SkillDetailsView->SetStructureData(StructData);
            }

            FSkillShowData* SkillShowData = nullptr;
            if (SkillShowDetailsView.IsValid())
            {
                SkillShowData = DataTableManager::Data<FSkillShowData>(NewSelectionName);
                if (SkillShowData)
                {
                    TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(FSkillShowData::StaticStruct(), (uint8*)SkillShowData));
                    SkillShowDetailsView->SetStructureData(StructData);
                }
            }            

            if (OnSelectAction.IsBound())
            {
                OnSelectAction.ExecuteIfBound(NewSelectionName, SkillData, SkillShowData);
            }
        }
    }
}

void SSkillDataDetailsWidget::OnTabChanged(int32 TabIndex)
{
    // 탭 변경 시 선택된 탭 인덱스를 저장
    SelectedTabIndex = TabIndex;

    if (DetailsContentBox.IsValid())
    {
        if (TabIndex == 0)
        {
            // 첫 번째 탭: Skill Details
            DetailsContentBox->SetContent(SkillDetailsView->GetWidget().ToSharedRef());
        }
        else if (TabIndex == 1)
        {
            // 두 번째 탭: Skill Show Details
            DetailsContentBox->SetContent(SkillShowDetailsView->GetWidget().ToSharedRef());
        }
    }
}