// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ActorPoolManager.h"
#include "ObjectPool/PooledActor.h"
#include "ObjectPool/ActorPoolCapacityDataAsset.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

void UActorPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
    FString PluginConfigPath = IPluginManager::Get().FindPlugin(TEXT("CoreUtils"))->GetBaseDir();
    FString ConfigFilePath = FPaths::Combine(PluginConfigPath, TEXT("Config/DefaultCoreUtils.ini"));

    // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
    FString AssetPath;
    if (GConfig)
    {
        GConfig->GetString(
            TEXT("ActorPoolSettings"),   // 섹션 이름
            TEXT("ActorPoolCapacityDataPath"),                  // 키 이름
            AssetPath,                                     // 값을 저장할 변수
            *ConfigFilePath                                // 플러그인의 ini 파일 경로
        );
    }

    // 경로가 유효하지 않은 경우 기본 경로 설정
    if (AssetPath.IsEmpty())
    {
        AssetPath = TEXT("/CoreUtils/ObjectPool/ActorPoolCapacityData.ActorPoolCapacityData");
    }

    // 풀 설정을 초기화
    InitializePoolSettings(AssetPath);
}

void UActorPoolManager::InitializePoolSettings(FString AssetPath)
{
    // 데이터 어셋을 로드
    UActorPoolCapacityDataAsset* ActorPoolCapacityData = LoadObject<UActorPoolCapacityDataAsset>(nullptr, *AssetPath);

    if (ActorPoolCapacityData)
    {
        // ActorPools 배열의 크기를 enum 값에 맞게 초기화
        ActorPools.SetNum(static_cast<int32>(EActorPoolType::Max));

        // 데이터 어셋을 기반으로 각 풀 타입에 대해 초기 용량을 설정
        for (const FPoolTypeSettings& PoolSetting : ActorPoolCapacityData->PoolSettings)
        {
            int32 PoolIndex = static_cast<int32>(PoolSetting.PoolType);

            // 풀의 초기 용량을 설정
            ActorPools[PoolIndex].SetNum(PoolSetting.InitialCapacity);

            // 풀을 확장하여 초기 용량만큼 액터를 생성하여 추가
            ExpandPool(PoolSetting.PoolType, PoolSetting.InitialCapacity);
        }

        // ActorPoolCapacityData는 더 이상 필요하지 않으므로 참조를 끊음
        ActorPoolCapacityData = nullptr;  // 참조를 끊으면 가비지 컬렉션에 의해 자동 해제됨
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load the pool settings data asset."));
    }
}

template <typename T>
T* UActorPoolManager::GetPooledObject(EActorPoolType ActorType)
{
    EnsurePoolsInitialized(ActorType);

    int32 Index = static_cast<int32>(ActorType);
    if (ActorPools[Index].Num() == 0)
    {
        ExpandPool(ActorType, 10);  // 필요시 풀 확장
    }

    AActor* PooledActor = ActorPools[Index].Pop();
    PooledActor->SetActorHiddenInGame(false);  // 객체 활성화

    // 객체가 IPooledActor 인터페이스를 구현했는지 확인
    checkf(PooledActor->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnPooled(PooledActor);

    return Cast<T>(PooledActor);
}

template <typename T>
void UActorPoolManager::ReturnPooledObject(T* Object, EActorPoolType ActorType)
{
    EnsurePoolsInitialized(ActorType);

    int32 Index = static_cast<int32>(ActorType);
    Object->SetActorHiddenInGame(true);  // 객체 비활성화

    // 객체가 IPooledActor 인터페이스를 구현했는지 확인
    checkf(Object->GetClass()->ImplementsInterface(UPooledActor::StaticClass()),
        TEXT("The pooled actor does not implement the IPooledActor interface."));

    IPooledActor::Execute_OnReturnedToPool(Object);

    ActorPools[Index].Add(Object);  // 풀에 객체를 다시 추가
}

void UActorPoolManager::ExpandPool(EActorPoolType ActorType, int32 PoolSize)
{
    UWorld* World = GetWorld();
    int32 Index = static_cast<int32>(ActorType);
    for (int32 i = 0; i < PoolSize; i++)
    {
        // 액터 클래스는 ActorType에 맞게 정의된 클래스 타입이어야 함
        AActor* NewActor = World->SpawnActor<AActor>(/* 적절한 액터 클래스를 스폰 */);
        NewActor->SetActorHiddenInGame(true);  // 비활성화된 상태로 추가
        ActorPools[Index].Add(NewActor);
    }
}