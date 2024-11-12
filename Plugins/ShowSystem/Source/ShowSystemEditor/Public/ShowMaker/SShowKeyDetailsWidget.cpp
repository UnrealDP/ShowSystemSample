// Fill out your copyright notice in the Description page of Project Settings.


#include "SShowKeyDetailsWidget.h"
#include "SlateOptMacros.h"
#include "RunTime/ShowBase.h"
#include "ShowMaker/ShowSequencerNotifyHook.h"
#include "RunTime/ShowSystem.h"
#include "ShowMaker/SShowCamSequenceDetailsWidget.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SShowKeyDetailsWidget::Construct(const FArguments& InArgs)
{
    EditorHelper = InArgs._InEditorHelper;
    ShowBasePtr = InArgs._InShowBasePtr;

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs ShowKeyDetailsViewArgs;
    ShowKeyDetailsViewArgs.bAllowSearch = true;
    ShowKeyDetailsViewArgs.bShowOptions = true;
    ShowKeyDetailsViewArgs.bHideSelectionTip = true;
    ShowKeyNotifyHookInstance = MakeShareable(new ShowSequencerNotifyHook(EditorHelper));
    ShowKeyDetailsViewArgs.NotifyHook = ShowKeyNotifyHookInstance.Get();

    FStructureDetailsViewArgs ShowKeyDetailsArgs;
    ShowKeyDetailsArgs.bShowObjects = true;

    ShowKeyStructureDetailsView = PropertyEditorModule.CreateStructureDetailView(
        ShowKeyDetailsViewArgs,
        ShowKeyDetailsArgs,
        nullptr,
        FText::FromString("Show Key Details")
    );

    ChildSlot
        [
            GetWidget()
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

TSharedRef<SWidget> SShowKeyDetailsWidget::GetWidget()
{
    ShowCamSequenceDetailsWidget = nullptr;

    if (ShowBasePtr)
	{
        switch (ShowBasePtr->GetKeyType())
        {
            case EShowKeyType::ShowKey_CamSequence:
            {
                ShowCamSequenceDetailsWidget = SNew(SShowCamSequenceDetailsWidget)
                    .ShowKeyStructureDetailsView(ShowKeyStructureDetailsView);
                return ShowCamSequenceDetailsWidget.ToSharedRef();
            }
            default:
                break;
        }
	}

    return SNew(SScrollBox)
        + SScrollBox::Slot()
        [
            ShowKeyStructureDetailsView.IsValid()
                ? ShowKeyStructureDetailsView->GetWidget().ToSharedRef()
                : SNullWidget::NullWidget
        ];
}

void SShowKeyDetailsWidget::UpdateEditorHelper(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper)
{
    EditorHelper = InEditorHelper;
    ShowKeyNotifyHookInstance->UpdateEditorHelper(EditorHelper);
}

void SShowKeyDetailsWidget::SetShowKey(TSharedPtr<FShowSequencerEditorHelper> InEditorHelper, UShowBase* InShowBasePtr)
{
    if (ShowBasePtr == InShowBasePtr)
    {
        return;
    }

    ShowBasePtr = InShowBasePtr;
    UpdateEditorHelper(InEditorHelper);

    if (ShowBasePtr)
    {
        UScriptStruct* ScriptStruct = ShowSystem::GetShowKeyStaticStruct(ShowBasePtr->GetShowKey()->KeyType);
        FShowKey* ShowKeyPtr = const_cast<FShowKey*>(ShowBasePtr->GetShowKey());
        TSharedRef<FStructOnScope> StructData = MakeShareable(new FStructOnScope(ScriptStruct, (uint8*)ShowKeyPtr));
        ShowKeyStructureDetailsView->SetStructureData(StructData);
    }
    else
    {
        ShowKeyStructureDetailsView->SetStructureData(nullptr);
    }

    ChildSlot
        [
            GetWidget()
        ];
}

void SShowKeyDetailsWidget::UpdateCameraView(UWorld* InWorld, FVector Position, FRotator Rotator, const TArray<AActor*>& ActorsToHide)
{
    if (!ShowCamSequenceDetailsWidget || 
        !ShowBasePtr ||
        !(ShowBasePtr->GetKeyType() == EShowKeyType::ShowKey_CamSequence))
    {
        return;
    }

    ShowCamSequenceDetailsWidget->UpdateCameraImage(InWorld, Position, Rotator, ActorsToHide);
}