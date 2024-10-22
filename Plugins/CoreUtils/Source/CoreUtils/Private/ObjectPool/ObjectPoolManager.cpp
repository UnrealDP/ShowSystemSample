// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ObjectPoolManager.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/PathsUtil.h"

void UObjectPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("UObjectPoolManager::Initialize called again, skipping as it is already initialized."));
        return;
    }
    ON_SCOPE_EXIT
    {
        bIsInitialized = true;
    };

    Super::Initialize(Collection);
    
    UObjectPoolManager::GetPoolSettings(PoolSettings);

    // 풀의 초기 용량을 설정
    ObjectPools.SetNum(PoolSettings.Num());    
    for (int32 i = 0; i < PoolSettings.Num(); ++i)
    {
        ObjectPools[i].Reserve(PoolSettings[i].InitialCapacity);
    }

    UE_LOG(LogTemp, Log, TEXT("UObjectPoolManager Initialized successfully."));
}

void UObjectPoolManager::Deinitialize()
{
    // ObjectPools에 있는 모든 오브젝트 제거
    for (TArray<UObject*>& Pool : ObjectPools)
    {
        for (UObject* Object : Pool)
        {
            if (Object && IsValid(Object))
            {
                // 가비지 컬렉션을 위한 참조 해제
                Object->RemoveFromRoot();
                Object = nullptr;
            }
        }
    }

    // 풀 배열과 예약 배열 비우기
    ObjectPools.Empty();
    PoolSettings.Empty();

    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("UObjectPoolManager: Object pool cleaned up."));
}

void UObjectPoolManager::GetPoolSettings(TArray<FObjectPoolTypeSettings>& OutPoolSettings)
{
    // 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("CoreUtils"), TEXT("Config/DefaultCoreUtils.ini"));

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

    // 데이터 어셋을 로드
    UObjectPoolCapacityDataAsset* ObjectPoolCapacityData = LoadObject<UObjectPoolCapacityDataAsset>(nullptr, *AssetPath);

    if (ObjectPoolCapacityData)
    {
        int32 NumPools = ObjectPoolCapacityData->PoolSettings.Num();
        OutPoolSettings.SetNum(NumPools);

        for (const FObjectPoolTypeSettings& PoolTypeSetting : ObjectPoolCapacityData->PoolSettings)
        {
            int32 PoolIndex = static_cast<int32>(PoolTypeSetting.PoolType);

            if (PoolIndex < 0 || PoolIndex >= ObjectPoolCapacityData->PoolSettings.Num())
            {
                checkf(false, TEXT("UObjectPoolCapacityDataAsset PoolType is Invalid [ %lld ]"), (long long)PoolIndex);
                UE_LOG(LogTemp, Error, TEXT("UObjectPoolCapacityDataAsset PoolType is Invalid [ %lld ]"), (long long)PoolIndex);
                continue;
            }

            // 풀 설정을 저장
            OutPoolSettings[PoolIndex] = PoolTypeSetting;
        }

        ObjectPoolCapacityData = nullptr;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load the pool settings data asset."));
    }
}

void UObjectPoolManager::ReturnPooledObject(UObject* Object, EObjectPoolType ObjectType)
{
    // ObjectPools 배열이 초기화되었는지 확인
    checkf(ObjectPools.Num() > 0, TEXT("[UObjectPoolManager] ObjectPools 배열이 초기화되지 않았습니다. InitializePoolSettings 함수를 먼저 호출하세요."));

    // 지정한 ObjectType에 해당하는 풀 배열이 유효한지 확인
    checkf(ObjectPools.IsValidIndex(static_cast<int32>(ObjectType)), TEXT("[UObjectPoolManager] 지정된 ObjectType에 해당하는 풀 배열이 초기화되지 않았습니다."));

    int32 Index = static_cast<int32>(ObjectType);

    // Object가 PoolSettings[Index].ObjectClass 의 인스턴스인지 확인
    checkf(Object->IsA(PoolSettings[Index].ObjectClass),
        TEXT("The pooled Object is not an instance of the expected class type."));

    // 객체가 IPooled 인터페이스를 구현했는지 확인
    checkf(Object->GetClass()->ImplementsInterface(UPooled::StaticClass()),
        TEXT("The pooled Object does not implement the IPooled interface."));

    IPooled* PooledInterface = Cast<IPooled>(Object);
    if (PooledInterface)
    {
        PooledInterface->OnReturnedToPool();
    }

    ObjectPools[Index].Add(Object);  // 풀에 객체를 다시 추가
}