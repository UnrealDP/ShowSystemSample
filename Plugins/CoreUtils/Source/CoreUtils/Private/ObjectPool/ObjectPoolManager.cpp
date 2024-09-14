// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolManager.h"
#include "ObjectPool/Pooled.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

void UObjectPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolManager::Initialize called again, skipping as it is already initialized."));
        return;
    }

    // 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
    FString PluginConfigPath = IPluginManager::Get().FindPlugin(TEXT("CoreUtils"))->GetBaseDir();
    FString ConfigFilePath = FPaths::Combine(PluginConfigPath, TEXT("Config/DefaultCoreUtils.ini"));

    // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
    FString AssetPath;
    if (GConfig)
    {
        GConfig->GetString(
            TEXT("PoolSettings"),   // 섹션 이름
            TEXT("ObjectPoolCapacityDataPath"),                  // 키 이름
            AssetPath,                                     // 값을 저장할 변수
            *ConfigFilePath                                // 플러그인의 ini 파일 경로
        );
    }

    // 경로가 유효하지 않은 경우 기본 경로 설정
    if (AssetPath.IsEmpty())
    {
        AssetPath = TEXT("/CoreUtils/ObjectPool/ObjectPoolCapacityData");
    }

    // 풀 설정을 초기화
    InitializePoolSettings(AssetPath);

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("UObjectPoolManager Initialized successfully."));
}

void UObjectPoolManager::Deinitialize()
{
    // ObjectPools에 있는 모든 액터 제거
    for (TArray<UObject*>& Pool : ObjectPools)
    {
        for (UObject* Object : Pool)
        {
            if (Object && IsValid(Object))
            {
                // 가비지 컬렉션을 위한 참조 해제
                Object = nullptr;
            }
        }
    }

    // 풀 배열과 예약 배열 비우기
    ObjectPools.Empty();
    PoolSettings.Empty();

    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("UObjectPoolManager: Object pool cleaned up."));
}

void UObjectPoolManager::InitializePoolSettings(FString AssetPath)
{
    // 데이터 어셋을 로드
    UObjectPoolCapacityDataAsset* ObjectPoolCapacityData = LoadObject<UObjectPoolCapacityDataAsset>(nullptr, *AssetPath);

    if (ObjectPoolCapacityData)
    {
        int32 NumPools = ObjectPoolCapacityData->PoolSettings.Num();
        PoolSettings.SetNum(NumPools);
        ObjectPools.SetNum(NumPools);

        for (const FObjectPoolTypeSettings& PoolTypeSetting : ObjectPoolCapacityData->PoolSettings)
        {
            int32 PoolIndex = static_cast<int32>(PoolTypeSetting.PoolType);

            // 풀 설정을 저장
            PoolSettings[PoolIndex] = PoolTypeSetting;

            // 풀의 초기 용량을 설정
            ObjectPools[PoolIndex].Reserve(PoolTypeSetting.InitialCapacity);
        }

        // ObjectPoolCapacityData 는 더 이상 필요하지 않으므로 참조를 끊음
        ObjectPoolCapacityData = nullptr;  // 참조를 끊으면 가비지 컬렉션에 의해 자동 해제됨
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load the pool settings data asset."));
    }
}

template <typename T>
T* UObjectPoolManager::GetPooledObject(EObjectPoolType ObjectType)
{
    EnsurePoolsInitialized(ObjectType);

    UObject* PooledObject;
    int32 Index = static_cast<int32>(ObjectType);
    if (ObjectPools[Index].Num() == 0)
    {
        ExpandPool(ObjectType);  // 필요시 풀 확장
    }

    PooledObject = ObjectPools[Index].Pop();

    // 객체가 IPooled 인터페이스를 구현했는지 확인
    checkf(PooledObject->GetClass()->ImplementsInterface(UPooled::StaticClass()),
        TEXT("The pooled Object does not implement the IPooled interface."));

    IPooled::Execute_OnPooled(PooledObject);

    return Cast<T>(PooledObject);
}

template <typename T>
void UObjectPoolManager::ReturnPooledObject(T* Object, EObjectPoolType ObjectType)
{
    EnsurePoolsInitialized(ObjectType);

    int32 Index = static_cast<int32>(ObjectType);

    // Object가 PoolSettings[Index].ObjectClass 의 인스턴스인지 확인
    checkf(Object->IsA(PoolSettings[Index].ObjectClass),
        TEXT("The pooled Object is not an instance of the expected class type."));

    // 객체가 IPooled 인터페이스를 구현했는지 확인
    checkf(Object->GetClass()->ImplementsInterface(UPooled::StaticClass()),
        TEXT("The pooled Object does not implement the IPooled interface."));

    IPooled::Execute_OnReturnedToPool(Object);

    ObjectPools[Index].Add(Object);  // 풀에 객체를 다시 추가
}

void UObjectPoolManager::ExpandPool(EObjectPoolType ObjectType)
{
    int32 Index = static_cast<int32>(ObjectType);
    int32 ReservedObjectCount = PoolSettings[Index].ReservedObjectCount;

    for (int32 i = 0; i < ReservedObjectCount; i++)
    {
        // Object 클래스는 ObjectType에 맞게 정의된 클래스 타입이어야 함
        UObject* Object = NewObject<UObject>(PoolSettings[Index].ObjectClass);  // UObject를 직접 생성
        if (Object)
        {
            ObjectPools[Index].Add(Object);
        }
    }
}
