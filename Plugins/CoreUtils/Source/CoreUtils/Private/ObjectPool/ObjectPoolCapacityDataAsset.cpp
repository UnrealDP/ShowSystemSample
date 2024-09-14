// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolCapacityDataAsset.h"

#include "ObjectPool/Pooled.h"

#if WITH_EDITOR
void UObjectPoolCapacityDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    for (FObjectPoolTypeSettings& Settings : PoolSettings)
    {
        if (Settings.ObjectClass)
        {
            // 객체가 UObject 를 상속받았는지 확인
            if (!Settings.ObjectClass->IsChildOf(UObject::StaticClass()))
            {
                // ObjectClass가 UObject 상속이 아니면 경고 팝업 표시
                FText WarningMessage = FText::FromString(TEXT("ObjectClass must inherit from UObject. Please check the class type."));
                FMessageDialog::Open(EAppMsgType::Ok, WarningMessage);

                Settings.ObjectClass = nullptr;
            }

            // 객체가 IPooled 인터페이스를 구현했는지 확인
            if (!Settings.ObjectClass->ImplementsInterface(UPooled::StaticClass()))
            {
                // ObjectClass가 IPooled 인터페이스를 구현하지 않으면 경고 팝업 표시
                FText WarningMessage = FText::FromString(TEXT("ObjectClass must implement IPooled interface. It has been adjusted automatically."));
                FMessageDialog::Open(EAppMsgType::Ok, WarningMessage);

                Settings.ObjectClass = nullptr;
            }
        }

        if (Settings.InitialCapacity < Settings.ReservedObjectCount)
        {
            // InitialCapacity가 ReservedActorCount보다 작으면 경고 팝업 표시
            FText WarningMessage = FText::FromString(TEXT("InitialCapacity must be greater than ReservedObjectCount. It has been adjusted automatically."));
            FMessageDialog::Open(EAppMsgType::Ok, WarningMessage);

            // 값을 자동으로 수정
            Settings.InitialCapacity = Settings.ReservedObjectCount;
        }
    }
}
#endif
