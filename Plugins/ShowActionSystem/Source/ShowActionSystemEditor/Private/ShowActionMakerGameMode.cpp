// Fill out your copyright notice in the Description page of Project Settings.


#include "ShowActionMakerGameMode.h"
#include "EDataTable.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
#include "Misc/PathsUtil.h"
#include "ActionComponent.h"
#include "DataTableManager.h"
#include "ShowActionSystemEditor.h"
#include "ActionServerExecutor.h"
#include "ShowActionSystemEditor.h"
#include "RunTime/ShowSequencerComponent.h"
#include "SSkillDataDetailsWidget.h"
#include "RunTime/ShowSequencer.h"
#include "ActionServerExecutor.h"

AShowActionMakerGameMode::AShowActionMakerGameMode()
{
    // Enabling ticking for this GameMode class
    PrimaryActorTick.bCanEverTick = true;
}

void AShowActionMakerGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (DefaultActorClass)
    {
        FVector CasterPos;
        FVector TargetPos;
        FRotator TargetRotator;
        GetPos(CasterPos, TargetPos, TargetRotator);

        FActorSpawnParameters CasterSpawnParams;
        CasterSpawnParams.Name = FName(TEXT("ActionMaker Caster"));

        Caster = GetWorld()->SpawnActor<AActor>(DefaultActorClass, CasterPos, FRotator::ZeroRotator, CasterSpawnParams);
        if (!Caster->FindComponentByClass<UShowSequencerComponent>())
        {
            UShowSequencerComponent* ShowSequencerComponent = NewObject<UShowSequencerComponent>(Caster, UShowSequencerComponent::StaticClass());
            Caster->AddInstanceComponent(ShowSequencerComponent);
            ShowSequencerComponent->RegisterComponent();
        }
        if (!Caster->FindComponentByClass<UActionComponent>())
        {
            UActionComponent* ActionComponent = NewObject<UActionComponent>(Caster, UActionComponent::StaticClass());
            Caster->AddInstanceComponent(ActionComponent);
			ActionComponent->RegisterComponent();
        }

        FActorSpawnParameters TargetSpawnParams;
        TargetSpawnParams.Name = FName(TEXT("ActionMaker Target"));

        AActor* SpawnedTarget = GetWorld()->SpawnActor<AActor>(DefaultActorClass, TargetPos, TargetRotator, TargetSpawnParams);
        if (SpawnedTarget)
        {
            if (!SpawnedTarget->FindComponentByClass<UShowSequencerComponent>())
            {
                UShowSequencerComponent* ShowSequencerComponent = NewObject<UShowSequencerComponent>(SpawnedTarget, UShowSequencerComponent::StaticClass());
                SpawnedTarget->AddInstanceComponent(ShowSequencerComponent);
                ShowSequencerComponent->RegisterComponent();
            }
            if (!SpawnedTarget->FindComponentByClass<UActionComponent>())
            {
                UActionComponent* ActionComponent = NewObject<UActionComponent>(SpawnedTarget, UActionComponent::StaticClass());
                SpawnedTarget->AddInstanceComponent(ActionComponent);
                ActionComponent->RegisterComponent();
            }

            Targets.Add(SpawnedTarget);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DefaultActorClass is not set!"));
    }

    if (FShowActionSystemEditor* ShowActionSystemEditorModulePtr = static_cast<FShowActionSystemEditor*>(FModuleManager::Get().GetModule(TEXT("ShowActionSystemEditor"))))
    {
        ShowActionSystemEditorModulePtr->InitializeModule(this);
    }
}

void AShowActionMakerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    SaveActorPositions();

    if (FShowActionSystemEditor* ShowActionSystemEditorModulePtr = static_cast<FShowActionSystemEditor*>(FModuleManager::Get().GetModule(TEXT("ShowActionSystemEditor"))))
    {
        ShowActionSystemEditorModulePtr->ClearModule();
    }

    if (CrrActionPtr)
    {
        DisposeAction();
    }

    if (Caster && !Caster->IsPendingKillPending())
    {
        Caster->Destroy();
        Caster = nullptr;
    }

    for (TObjectPtr<AActor>& Target : Targets)
    {
        if (Target && !Target->IsPendingKillPending())
        {
            Target->Destroy();
            Target = nullptr;
        }
    }
    Targets.Empty();

    SkillData = nullptr;
    SkillShowData = nullptr;

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

UActionBase* AShowActionMakerGameMode::SelectAction(
    FName InSelectedActionName, 
    FSkillData* InSkillData, 
    FSkillShowData* InSkillShowData,
    UShowSequencer*& OutCastShowSequencer,
    UShowSequencer*& OutExecShowSequencer,
    UShowSequencer*& OutFinishShowSequencer)
{
    SelectedActionName = InSelectedActionName;
    SkillData = InSkillData;
    SkillShowData = InSkillShowData;

    if (Caster)
    {
        if (CrrActionPtr)
        {
            if (CrrActionPtr->GetActionName().IsEqual(InSelectedActionName))
            {
                return CrrActionPtr;
            }
			else
			{
				DisposeAction();
			}
        }

        UActionComponent* ActionComponent = Caster->FindComponentByClass<UActionComponent>();       
        CrrActionPtr = NewObject<UActionServerExecutor>(ActionComponent);
        CrrActionPtr->ActionName = SelectedActionName;
        CrrActionPtr->Owner = Caster;
        CrrActionPtr->ActionBaseData = SkillData;
        CrrActionPtr->ActionBaseShowData = SkillShowData;
        CrrActionPtr->State = EActionState::Wait;
        CrrActionPtr->StepPassedTime = 0.0f;
        CrrActionPtr->RemainCoolDown = 0.0f;
        CrrActionPtr->ShowPlayer = Caster->GetWorld()->GetSubsystem<UShowPlayer>();
        CrrActionPtr->SetDontDestroy();
        CrrActionPtr->AddToRoot();

        OutCastShowSequencer = CrrActionPtr->NewShowSequencer(EActionState::Cast);
        OutExecShowSequencer = CrrActionPtr->NewShowSequencer(EActionState::Exec);
        OutFinishShowSequencer = CrrActionPtr->NewShowSequencer(EActionState::Finish);

        if (OutCastShowSequencer)
        {
            OutCastShowSequencer->SetDontDestroy();
        }
        if (OutExecShowSequencer)
        {
            OutExecShowSequencer->SetDontDestroy();
        }
        if (OutFinishShowSequencer)
        {
            OutFinishShowSequencer->SetDontDestroy();
        }

        ActionComponent->ActiveActions.Add(CrrActionPtr->GetActionName(), CrrActionPtr);
    }

    return CrrActionPtr;
}

void AShowActionMakerGameMode::DisposeAction()
{
    if (!CrrActionPtr)
    {
        return;
    }

    if (!Caster)
    {
        return;
    }
    
    if (CrrActionPtr->ShowPlayer)
    {
        if (CrrActionPtr->CastShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(CrrActionPtr->Owner, CrrActionPtr->CastShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(CrrActionPtr->Owner, CrrActionPtr->CastShowPtr);
            }
            CrrActionPtr->CastShowPtr = nullptr;
        }

        if (CrrActionPtr->ExecShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(CrrActionPtr->Owner, CrrActionPtr->ExecShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(CrrActionPtr->Owner, CrrActionPtr->ExecShowPtr);
            }
            CrrActionPtr->ExecShowPtr = nullptr;
        }

        if (CrrActionPtr->FinishShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(CrrActionPtr->Owner, CrrActionPtr->FinishShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(CrrActionPtr->Owner, CrrActionPtr->FinishShowPtr);
            }
            CrrActionPtr->FinishShowPtr = nullptr;
        }
    }

    UActionComponent* ActionComponent = Caster->FindComponentByClass<UActionComponent>();
    if (CrrActionPtr == ActionComponent->MainActionPtr)
    {
        ActionComponent->MainActionPtr = nullptr;
    }

    UActionBase** ActionBasePtrAddress = ActionComponent->ActiveActions.Find(CrrActionPtr->GetActionName());
    if (ActionBasePtrAddress)
    {
        UActionBase* ActionPtr = *ActionBasePtrAddress;
        ActionComponent->ActiveActions.Remove(ActionPtr->GetActionName());

        ActionPtr->Cancel();        
        ActionPtr->OnReturnedToPool();
        ActionPtr->RemoveFromRoot();
    }
    CrrActionPtr = nullptr;
}

UShowSequencer* AShowActionMakerGameMode::ChangeShow(EActionState ActionState, FSoftObjectPath* NewShowPath)
{
    if (!CrrActionPtr)
    {
        return nullptr;
    }

    AActor* ActionOwner = CrrActionPtr->GetOwner();
    UShowSequencerComponent* ShowSequencerComponent = ActionOwner->FindComponentByClass<UShowSequencerComponent>();
    switch (ActionState)
    {
    case EActionState::Cast:
        if (CrrActionPtr->CastShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(ActionOwner, CrrActionPtr->CastShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(ActionOwner, CrrActionPtr->CastShowPtr);
            }
            CrrActionPtr->CastShowPtr = nullptr;
        }
        break;
    case EActionState::Exec:
        if (CrrActionPtr->ExecShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(ActionOwner, CrrActionPtr->ExecShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(ActionOwner, CrrActionPtr->ExecShowPtr);
            }
            CrrActionPtr->ExecShowPtr = nullptr;
        }
        break;
    case EActionState::Finish:
        if (CrrActionPtr->FinishShowPtr)
        {
            if (CrrActionPtr->ShowPlayer->HasShowSequencer(ActionOwner, CrrActionPtr->FinishShowPtr))
            {
                CrrActionPtr->ShowPlayer->DisposeSoloShow(ActionOwner, CrrActionPtr->FinishShowPtr);
            }
            CrrActionPtr->FinishShowPtr = nullptr;
        }
        break;
    default:
        break;
    }

    UShowSequencer* NewShowSequencerPtr = nullptr;
    if (NewShowPath && NewShowPath->IsValid())
    {
        NewShowSequencerPtr = CrrActionPtr->NewShowSequencer(ActionState);
    }
    return NewShowSequencerPtr;
}

void AShowActionMakerGameMode::DoAction()
{
	if (Caster && CrrActionPtr)
	{
		UActionComponent* ActionComponent = Caster->FindComponentByClass<UActionComponent>();
        ActionComponent->MainActionPtr = nullptr;

        if (UActionBase* Action = ActionComponent->GetActiveAction(CrrActionPtr->GetActionName()))
        {
            Action->Reset();
        }
        else
        {
            ActionComponent->ActiveActions.Add(CrrActionPtr->GetActionName(), CrrActionPtr);
        }
            	
        CrrActionPtr->DoAction();
	}
}
