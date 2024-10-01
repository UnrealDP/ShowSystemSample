// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowActionMakerGameMode.h"
#include "DataTableManager.h"
#include "EDataTable.h"
#include "Data/SkillData.h"
#include "Misc/PathsUtil.h"
#include "ActionComponent.h"

AShowActionMakerGameMode::AShowActionMakerGameMode()
{
    // Enabling ticking for this GameMode class
    PrimaryActorTick.bCanEverTick = true;
}

void AShowActionMakerGameMode::BeginPlay()
{
    Super::BeginPlay();

    DataTableManager::Get().InitializeDataTables({ EDataTable::SkillData });
    SkillDataTable = DataTableManager::DataTable(EDataTable::SkillData);
    if (!SkillDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("SkillDataTable is null"));
    }

    if (DefaultActorClass)
    {
        FVector CasterPos;
        FVector TargetPos;
        FRotator TargetRotator;
        GetPos(CasterPos, TargetPos, TargetRotator);

        Caster = GetWorld()->SpawnActor<AActor>(DefaultActorClass, CasterPos, FRotator::ZeroRotator);
        if (!Caster->FindComponentByClass<UActionComponent>())
        {
            UActionComponent* ActionComponent = NewObject<UActionComponent>(Caster, UActionComponent::StaticClass());
			ActionComponent->RegisterComponent();
        }

        AActor* SpawnedTarget = GetWorld()->SpawnActor<AActor>(DefaultActorClass, TargetPos, TargetRotator);
        if (SpawnedTarget)
        {
            if (!SpawnedTarget->FindComponentByClass<UActionComponent>())
            {
                UActionComponent* ActionComponent = NewObject<UActionComponent>(SpawnedTarget, UActionComponent::StaticClass());
                ActionComponent->RegisterComponent();
            }

            Targets.Add(SpawnedTarget);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DefaultActorClass is not set!"));
    }
}

void AShowActionMakerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (Caster && !Caster->IsPendingKillPending())
    {
        Caster->Destroy();
        Caster = nullptr;
    }

    for (AActor* Target : Targets)
	{
		if (Target && !Target->IsPendingKillPending())
		{
			Target->Destroy();
            Target = nullptr;
		}
	}
    Targets.Empty();

    SkillDataTable = nullptr;
    DataTableManager::Destroy();
    SaveActorPositions();

    UE_LOG(LogTemp, Log, TEXT("GameMode EndPlay called. Reason: %d"), static_cast<int32>(EndPlayReason));
}


void AShowActionMakerGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AShowActionMakerGameMode::GetPos(FVector& CasterPos, FVector& TargetPos, FRotator& TargetRotator)
{
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowActionSystem"), TEXT("Config/ShowActionMaker.ini"));

    if (!GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("CasterDefaultLocation"), CasterPos, *ConfigFilePath))
    {
        // 기본 위치값 설정 (최초 실행 시)
        CasterPos = FVector(500.0f, 1000.0f, 50.0f);
    }

    if (!GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultLocation"), TargetPos, *ConfigFilePath))
    {
        // 기본 타겟 위치값 설정 (최초 실행 시)
        TargetPos = FVector(500.0f, 1500.0f, 50.0f);
    }

    if (!GConfig->GetRotator(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultRotation"), TargetRotator, *ConfigFilePath))
    {
        // 기본 타겟 회전값 설정 (최초 실행 시)
        TargetRotator = FRotator(0.0f, 180.0f, 0.0f);
    }
}

void AShowActionMakerGameMode::SaveActorPositions()
{
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowActionSystem"), TEXT("Config/ShowActionMaker.ini"));

    FVector ConfigCasterLocation;
    FVector ConfigTargetLocation;
    FRotator ConfigTargetRotator;

    GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("CasterDefaultLocation"), ConfigCasterLocation, *ConfigFilePath);
    GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultLocation"), ConfigTargetLocation, *ConfigFilePath);
    GConfig->GetRotator(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultRotation"), ConfigTargetRotator, *ConfigFilePath);

    bool bIsUpdate = false;
    if (!Caster->GetActorLocation().Equals(ConfigCasterLocation, KINDA_SMALL_NUMBER))
    {
        FVector NewCasterLocation = Caster->GetActorLocation();
        GConfig->SetVector(TEXT("AShowActionMakerGameMode"), TEXT("CasterDefaultLocation"), NewCasterLocation, *ConfigFilePath);
        bIsUpdate = true;
    }

    if (!Targets[0]->GetActorLocation().Equals(ConfigTargetLocation, KINDA_SMALL_NUMBER))
    {
        FVector NewTargetLocation = Targets[0]->GetActorLocation();
        GConfig->SetVector(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultLocation"), NewTargetLocation, *ConfigFilePath);
        bIsUpdate = true;
    }

    if (!Targets[0]->GetActorRotation().Equals(ConfigTargetRotator, KINDA_SMALL_NUMBER))
    {
        FRotator NewTargetRotator = Targets[0]->GetActorRotation();
        GConfig->SetRotator(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultRotation"), NewTargetRotator, *ConfigFilePath);
        bIsUpdate = true;
    }

    if (bIsUpdate)
    {
        GConfig->Flush(false, *ConfigFilePath);
    }
}

void AShowActionMakerGameMode::SelectAction(const FName& ActionName)
{
    if (Caster)
    {
        UActionComponent* ActionComponent = Caster->FindComponentByClass<UActionComponent>();
        ActionComponent->InitializeActionPool({ ActionName });
    }
}

void AShowActionMakerGameMode::DoAction()
{
	if (Caster)
	{
		UActionComponent* ActionComponent = Caster->FindComponentByClass<UActionComponent>();
		ActionComponent->DoActionPool(FName("Skill_1"));
	}
}