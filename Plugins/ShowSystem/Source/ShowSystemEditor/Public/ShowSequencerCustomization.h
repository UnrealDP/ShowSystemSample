// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

/**
 * 
 */
class FShowSequencerCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance();

    // 이 메서드에서 커스터마이징할 속성들을 정의
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    //void OnStructSelected_New(TSharedPtr<IPropertyHandle> ArrayElement, UScriptStruct* NewSelection);
    void OnStructSelected(UScriptStruct* NewSelection, ESelectInfo::Type SelectInfo);
    FReply OnAddButtonClicked();
    FReply OnEmptyButtonClicked();

    IDetailLayoutBuilder* CachedDetailBuilder;
    TSharedPtr<IPropertyHandleArray> ShowKeysArrayHandle;
    UScriptStruct* SelectedStruct = nullptr;
};