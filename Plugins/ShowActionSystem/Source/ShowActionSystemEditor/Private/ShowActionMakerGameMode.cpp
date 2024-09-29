// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowActionMakerGameMode.h"
#include "DataTableManager.h"
#include "EDataTable.h"
#include "Data/SkillData.h"

AShowActionMakerGameMode::AShowActionMakerGameMode()
{
    // Enabling ticking for this GameMode class
    PrimaryActorTick.bCanEverTick = true;
}

void AShowActionMakerGameMode::BeginPlay()
{
    Super::BeginPlay();

    DataTableManager::Get().InitializeDataTables({ EDataTable::SkillData });
    UDataTable* SkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);

    if (SkillDataTable)
    {
        // 데이터 테이블에서 원하는 RowKey로 데이터를 가져옴
        static const FString ContextString(TEXT("SkillDataContext"));
        FName RowKey = FName(TEXT("test_skill_1"));

        FSkillData* SkillData = SkillDataTable->FindRow<FSkillData>(RowKey, ContextString, true);
        if (SkillData)
        {
            UE_LOG(LogTemp, Log, TEXT("Skill Name: %s, CastDuration: %f"), *SkillData->Name, SkillData->CastDuration);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Row not found in SkillDataTable with key: %s"), *RowKey.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SkillDataTable is null"));
    }
}

void AShowActionMakerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    DataTableManager::Destroy();

    UE_LOG(LogTemp, Log, TEXT("GameMode EndPlay called. Reason: %d"), static_cast<int32>(EndPlayReason));
}


void AShowActionMakerGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}
