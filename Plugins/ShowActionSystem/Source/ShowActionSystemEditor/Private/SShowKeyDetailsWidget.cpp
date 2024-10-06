// Fill out your copyright notice in the Description page of Project Settings.


#include "SShowKeyDetailsWidget.h"
#include "SlateOptMacros.h"
#include "RunTime/ShowBase.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyDetailsWidget::Construct(const FArguments& InArgs)
{
	SelectedShowKey = InArgs._ShowKey;

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = true;
    DetailsViewArgs.bShowOptions = true;

    FStructureDetailsViewArgs StructureDetailsArgs;

    // 레이아웃 생성
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    // 스킬 데이터 편집을 위한 구조체 디테일 뷰
                    StructureDetailsView.IsValid() ? StructureDetailsView->GetWidget().ToSharedRef() : SNullWidget::NullWidget
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SShowKeyDetailsWidget::SetSkillKey(FShowKey* NewShowKey)
{
    if (SelectedShowKey == NewShowKey)
    {
        return;
    }

    if (SelectedShowKey)
    {
        // 구조체 데이터
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(FShowKey::StaticStruct(), (uint8*)SelectedShowKey));

        // StructureDetailsView 생성
        FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

        StructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
            FDetailsViewArgs(),
            FStructureDetailsViewArgs(),
            StructData,
            FText::FromString("ShowKey Details")
        );

        StructureDetailsView->SetStructureData(StructData);
    }
    else
    {
		StructureDetailsView = nullptr;
	}
}