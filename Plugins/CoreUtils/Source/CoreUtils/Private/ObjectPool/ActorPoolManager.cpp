// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPool/ActorPoolManager.h"
#include "ObjectPool/Pooled.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "Misc/PathsUtil.h"

void UActorPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("UActorPoolManager::Initialize called again, skipping as it is already initialized."));
        return;
    }

    // 플러그인 폴더 경로에서 설정 파일 경로를 가져오기
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("CoreUtils"), TEXT("Config/DefaultCoreUtils.ini"));

    // 풀 설정 데이터 어셋의 경로를 ini 파일에서 읽어오기
    FString AssetPath;
    if (GConfig)
    {
        GConfig->GetString(
            TEXT("PoolSettings"),   // 섹션 이름
            TEXT("ActorPoolCapacityDataPath"),                  // 키 이름
            AssetPath,                                     // 값을 저장할 변수
            *ConfigFilePath                                // 플러그인의 ini 파일 경로
        );
    }

    // 경로가 유효하지 않은 경우 기본 경로 설정
    if (AssetPath.IsEmpty())
    {
        AssetPath = TEXT("/CoreUtils/ObjectPool/ActorPoolCapacityData");
    }

    // 풀 설정을 초기화
    InitializePoolSettings(AssetPath);

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("UActorPoolManager Initialized successfully."));
}

void UActorPoolManager::Deinitialize()
{
    // ActorPools에 있는 모든 액터 제거
    for (TArray<AActor*>& Pool : ActorPools)
    {
        for (AActor* Actor : Pool)
        {
            if (Actor && IsValid(Actor))
            {
                // 액터를 명시적으로 제거하여 메모리 해제
                Actor->Destroy();
            }
        }
    }

    // 풀 배열과 예약 배열 비우기
    ActorPools.Empty();
    PoolSettings.Empty();

    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("UActorPoolManager: Actor pool cleaned up."));
}

void UActorPoolManager::InitializePoolSettings(FString AssetPath)
{
    // 데이터 어셋을 로드
    UActorPoolCapacityDataAsset* ActorPoolCapacityData = LoadObject<UActorPoolCapacityDataAsset>(nullptr, *AssetPath);

    if (ActorPoolCapacityData)
    {
        int32 NumPools = ActorPoolCapacityData->PoolSettings.Num();
        PoolSettings.SetNum(NumPools);
        ActorPools.SetNum(NumPools);

        for(const FActorPoolTypeSettings& PoolTypeSetting : ActorPoolCapacityData->PoolSettings)
		{
            int32 PoolIndex = static_cast<int32>(PoolTypeSetting.PoolType);

            // 풀 설정을 저장
            PoolSettings[PoolIndex] = PoolTypeSetting;

            // 풀의 초기 용량을 설정
            ActorPools[PoolIndex].Reserve(PoolTypeSetting.InitialCapacity);
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
T* UActorPoolManager::GetPooledObject(EActorPoolType ActorType, const FActorSpawnParameters& SpawnParameters)
{
    GetPooledObject(ActorType, NULL, SpawnParameters);
}

template <typename T>
T* UActorPoolManager::GetPooledObject(EActorPoolType ActorType, FVector const& Location, FRotator const& Rotation, const FActorSpawnParameters& SpawnParameters)
{
    FTransform Transform;
    if (Location)
    {
        Transform.SetLocation(*Location);
    }
    if (Rotation)
    {
        Transform.SetRotation(FQuat(*Rotation));
    }

    GetPooledObject(ActorType, Transform, SpawnParameters);
}

template <typename T>
T* UActorPoolManager::GetPooledObject(EActorPoolType ActorType, FTransform const& Transform, const FActorSpawnParameters& SpawnParameters)
{
    EnsurePoolsInitialized(ActorType);

    AActor* PooledActor;
    int32 Index = static_cast<int32>(ActorType);
    if (ActorPools[Index].Num() == 0)
    {
        ExpandPool(ActorType, Transform, SpawnParameters);  // 필요시 풀 확장
        PooledActor = ActorPools[Index].Pop();
    }
    else
    {
        PooledActor = ActorPools[Index].Pop();
        if (SpawnParameters)
        {
            UpdateSpawnParameters(PooledActor, SpawnParameters);
        }
    }
    PooledActor->SetActorHiddenInGame(false);  // 객체 활성화

    // 객체가 IPooled 인터페이스를 구현했는지 확인
    checkf(PooledActor->GetClass()->ImplementsInterface(UPooled::StaticClass()),
        TEXT("The pooled actor does not implement the IPooled interface."));

    IPooled::Execute_OnPooled(PooledActor);

    return Cast<T>(PooledActor);
}

template <typename T>
void UActorPoolManager::ReturnPooledObject(T* Object, EActorPoolType ActorType)
{
    EnsurePoolsInitialized(ActorType);

    int32 Index = static_cast<int32>(ActorType);
    Object->SetActorHiddenInGame(true);  // 객체 비활성화

    // Object가 PoolSettings[Index].ActorClass 의 인스턴스인지 확인
    checkf(Object->IsA(PoolSettings[Index].ActorClass),
        TEXT("The pooled Object is not an instance of the expected class type."));

    // 객체가 IPooled 인터페이스를 구현했는지 확인
    checkf(Object->GetClass()->ImplementsInterface(UPooled::StaticClass()),
        TEXT("The pooled actor does not implement the IPooled interface."));

    IPooled::Execute_OnReturnedToPool(Object);

    ActorPools[Index].Add(Object);  // 풀에 객체를 다시 추가
}

void UActorPoolManager::ExpandPool(EActorPoolType ActorType, FTransform const& Transform, const FActorSpawnParameters& SpawnParameters)
{
    UWorld* World = GetWorld();
    int32 Index = static_cast<int32>(ActorType);
    int32 ReservedActorCount = PoolSettings[Index].ReservedActorCount;

    for (int32 i = 0; i < ReservedActorCount; i++)
    {
        // 액터 클래스는 ActorType에 맞게 정의된 클래스 타입이어야 함
        AActor* NewActor = World->SpawnActor<AActor>(PoolSettings[Index].ActorClass, Transform, SpawnParameters);
        NewActor->SetActorHiddenInGame(true);  // 비활성화된 상태로 추가
        ActorPools[Index].Add(NewActor);
    }
}

// 액터의 스폰 파라미터를 업데이트하는 메서드
// 여기는 추후 사용하게 되면 필히 수정이 필요함
void UActorPoolManager::UpdateSpawnParameters(AActor* Actor, const FActorSpawnParameters& SpawnParameters)
{
    if (!Actor)
    {
        UE_LOG(LogTemp, Error, TEXT("Actor is null!"));
        return;
    }

    // 액터의 이름을 업데이트
    if (!SpawnParameters.Name.IsNone())
    {
        Actor->Rename(*SpawnParameters.Name.ToString());
    }

    // 소유자를 업데이트
    Actor->SetOwner(SpawnParameters.Owner);

    // Instigator 업데이트
    Actor->SetInstigator(SpawnParameters.Instigator);

    // OverrideLevel 업데이트 (직접 레벨을 이동시키는 추가 작업 필요)
    if (SpawnParameters.OverrideLevel)
    {
        // 실제로 액터를 다른 레벨로 이동시키는 작업 필요
        UE_LOG(LogTemp, Warning, TEXT("OverrideLevel is not automatically handled, additional code required."));
    }

    // 콜리전 처리 방식을 설정
    if (SpawnParameters.SpawnCollisionHandlingOverride != ESpawnActorCollisionHandlingMethod::Undefined)
    {
        // 스폰 후 콜리전 처리 방식을 변경할 수 있는지 확인 필요
        // 콜리전 처리 방식을 설정하는 코드가 필요할 수 있음
    }

    // 기타 스폰 파라미터들을 필요에 따라 처리
    if (SpawnParameters.OverrideParentComponent)
    {
        // 액터가 부모 컴포넌트가 있으면 설정 (필요 시 추가 코드)
        UE_LOG(LogTemp, Warning, TEXT("OverrideParentComponent is not automatically handled, additional code required."));
    }

    // Transform Scale Method 적용
    // 실제 액터의 스케일을 이 파라미터에 맞춰 업데이트하는 코드가 필요할 수 있음
}
