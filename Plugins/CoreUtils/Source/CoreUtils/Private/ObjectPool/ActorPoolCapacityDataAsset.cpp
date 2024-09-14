// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ActorPoolCapacityDataAsset.h"
#include "ObjectPool/Pooled.h"

#if WITH_EDITOR
void UActorPoolCapacityDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    for (FPoolTypeSettings& Settings : PoolSettings)
    {
        if (Settings.ActorClass)
        {
            // 객체가 IPooled 인터페이스를 구현했는지 확인
            if (!Settings.ActorClass->ImplementsInterface(UPooled::StaticClass()))
			{
				// ActorClass가 IPooled 인터페이스를 구현하지 않으면 경고 팝업 표시
				FText WarningMessage = FText::FromString(TEXT("ActorClass must implement IPooled interface. It has been adjusted automatically."));
				FMessageDialog::Open(EAppMsgType::Ok, WarningMessage);

				Settings.ActorClass = nullptr;
			}
        }

        if (Settings.InitialCapacity < Settings.ReservedActorCount)
        {
            // InitialCapacity가 ReservedActorCount보다 작으면 경고 팝업 표시
            FText WarningMessage = FText::FromString(TEXT("InitialCapacity must be greater than ReservedActorCount. It has been adjusted automatically."));
            FMessageDialog::Open(EAppMsgType::Ok, WarningMessage);

            // 값을 자동으로 수정
            Settings.InitialCapacity = Settings.ReservedActorCount;
        }
    }
}
#endif
