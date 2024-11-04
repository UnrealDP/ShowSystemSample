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
#include "Kismet/GameplayStatics.h"
#include "DebugCameraHelper.h"
#include "RunTime/ShowBase.h"
#include "RunTime/ShowKeys/ShowCamSequence.h"
#include "ShowMaker/ShowSequencerEditorHelper.h"
#include "Camera/CameraComponent.h"

AShowActionMakerGameMode::AShowActionMakerGameMode()
{
    // Enabling ticking for this GameMode class
    PrimaryActorTick.bCanEverTick = true;
}

void AShowActionMakerGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (DefaultPawnClass)
    {
        FVector CasterPos;
        FVector TargetPos;        
        FRotator CasterRotator;
        FRotator TargetRotator;
        GetPos(CasterPos, TargetPos, CasterRotator, TargetRotator);

        FActorSpawnParameters CasterSpawnParams;
        CasterSpawnParams.Name = FName(TEXT("ActionMaker Caster"));

        if (GEngine->GetWorldContextFromWorldChecked(GetWorld()).WorldType == EWorldType::PIE)
        {
            // 현재 플레이어의 Actor 가져오기
            APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PlayerController)
            {
                PlayerController->InputComponent->BindAction("FKeyAction", IE_Pressed, this, &AShowActionMakerGameMode::DoAction);
                PlayerController->InputComponent->BindAction("OnMouseLClick", IE_Pressed, this, &AShowActionMakerGameMode::OnMouseLClick);

                APawn* PlayerPawn = PlayerController->GetPawn();
                if (PlayerPawn)
                {
                    Caster = PlayerPawn;
                    Caster->Rename(*CasterSpawnParams.Name.ToString());
                    Caster->SetActorLocation(CasterPos);
                    Caster->SetActorRotation(CasterRotator);
                }
                else
                {
                    Caster = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, CasterPos, CasterRotator, CasterSpawnParams);
                    Caster->SetActorLabel(CasterSpawnParams.Name.ToString());
                    Caster->Rename(*CasterSpawnParams.Name.ToString());
                    Caster->SetActorLocation(CasterPos);
                    Caster->SetActorRotation(CasterRotator);
                    PlayerController->SetPawn(Caster);
                }
            }
        }
        if (!Caster)
        {
            Caster = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, CasterPos, CasterRotator, CasterSpawnParams);
            Caster->SetActorLabel(CasterSpawnParams.Name.ToString());
        }

        if (!Caster->FindComponentByClass<UShowSequencerComponent>())
        {
            UShowSequencerComponent* ShowSequencerComponent = NewObject<UShowSequencerComponent>(Caster, UShowSequencerComponent::StaticClass());
            Caster->AddInstanceComponent(ShowSequencerComponent);
            ShowSequencerComponent->RegisterComponent();
            ShowSequencerComponent->Initialize();
        }
        if (!Caster->FindComponentByClass<UActionComponent>())
        {
            UActionComponent* ActionComponent = NewObject<UActionComponent>(Caster, UActionComponent::StaticClass());
            Caster->AddInstanceComponent(ActionComponent);
			ActionComponent->RegisterComponent();
        }
        Caster->OnDestroyed.AddDynamic(this, &AShowActionMakerGameMode::OnActorDestroyed);

        FActorSpawnParameters TargetSpawnParams;
        TargetSpawnParams.Name = FName(TEXT("ActionMaker Target"));

        APawn* SpawnedTarget = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, TargetPos, TargetRotator, TargetSpawnParams);
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
            SpawnedTarget->OnDestroyed.AddDynamic(this, &AShowActionMakerGameMode::OnActorDestroyed);

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

void AShowActionMakerGameMode::Initialize(FOnUpdateCameraView InOnUpdateCameraView)
{
    OnUpdateCameraView = InOnUpdateCameraView;
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

    for (AActor* Target : Targets)
    {
        if (Target && !Target->IsPendingKillPending())
        {
            Target->Destroy();
        }
    }
    Targets.Empty();

    SkillData = nullptr;
    SkillShowData = nullptr;

    if (DebugCameraHelper)
    {
        DebugCameraHelper->Destroy();
        DebugCameraHelper = nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("GameMode EndPlay called. Reason: %d"), static_cast<int32>(EndPlayReason));
}

void AShowActionMakerGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

void AShowActionMakerGameMode::GetPos(FVector& CasterPos, FVector& TargetPos, FRotator& CasterRotator, FRotator& TargetRotator)
{
    FString ConfigFilePath = PathsUtil::PluginConfigPath(TEXT("ShowActionSystem"), TEXT("Config/ShowActionMaker.ini"));

    //if (!GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("CasterDefaultLocation"), CasterPos, *ConfigFilePath))
    {
        // 기본 위치값 설정 (최초 실행 시)
        CasterPos = FVector(1000.0f, 2000.0f, 80.0f);
    }

    //if (!GConfig->GetVector(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultLocation"), TargetPos, *ConfigFilePath))
    {
        // 기본 타겟 위치값 설정 (최초 실행 시)
        TargetPos = FVector(1500.0f, 2000.0f, 80.0f);
    }

    //if (!GConfig->GetRotator(TEXT("AShowActionMakerGameMode"), TEXT("CasterDefaultRotation"), CasterRotator, *ConfigFilePath))
    {
        // 기본 타겟 회전값 설정 (최초 실행 시)
        CasterRotator = FRotator(0.0f, 0.0f, 0.0f);
    }

    //if (!GConfig->GetRotator(TEXT("AShowActionMakerGameMode"), TEXT("TargetDefaultRotation"), TargetRotator, *ConfigFilePath))
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

void AShowActionMakerGameMode::OnActorDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor == Caster)
    {
        DisposeAction();
    }
}

void AShowActionMakerGameMode::SelectKey(TSharedPtr<FShowSequencerEditorHelper> InSelectedShowSequencerEditorHelper, UShowBase* InSelectedShowBasePtr)
{
    SelectedShowSequencerEditorHelper = InSelectedShowSequencerEditorHelper;
    SelectedShowBasePtr = InSelectedShowBasePtr;

    if (DebugCameraHelper)
    {
        DebugCameraHelper->Destroy();
        DebugCameraHelper = nullptr;
    }

    if (SelectedShowBasePtr && SelectedShowBasePtr->IsA(UShowCamSequence::StaticClass()))
    {
        DebugCameraHelper = GetWorld()->SpawnActor<ADebugCameraHelper>();
        DebugCameraHelper->Initialize(Caster, Cast<UShowCamSequence>(SelectedShowBasePtr));
        DebugCameraHelper->OnUpdate.BindUObject(this, &AShowActionMakerGameMode::ShowSequenceAssetMarkPackageDirty);
        DebugCameraHelper->GetCameraLocationDelegate.BindUObject(this, &AShowActionMakerGameMode::GetCameraLocation);
        DebugCameraHelper->OnUpdateCameraPathPoint.BindUObject(this, &AShowActionMakerGameMode::UpdateCameraPathPoint);
    }
}

void AShowActionMakerGameMode::ShowSequenceAssetMarkPackageDirty()
{
    if (SelectedShowSequencerEditorHelper->ShowSequenceAssetMarkPackageDirty() == false)
    {
        TObjectPtr<UShowSequenceAsset> ShowSequenceAsset = SelectedShowSequencerEditorHelper->GetShowSequenceAsset();
        UPackage* Package = ShowSequenceAsset->GetOutermost();

        if (Package != NULL)
        {
            const bool bIsDirty = Package->IsDirty();

            if (!bIsDirty)
            {
                Package->SetDirtyFlag(true);
            }

            Package->PackageMarkedDirtyEvent.Broadcast(Package, bIsDirty);
        }
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
    if (DebugCameraHelper)
    {
        DebugCameraHelper->Destroy();
        DebugCameraHelper = nullptr;
    }

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

void AShowActionMakerGameMode::OnMouseLClick()
{
    UE_LOG(LogTemp, Log, TEXT("OnMouseLClick"));
    if (DebugCameraHelper)
    {
    }
}

FVector AShowActionMakerGameMode::GetCameraLocation()
{
#if WITH_EDITOR
    if (GEditor->bIsSimulatingInEditor)
    {
        FEditorViewportClient* EditorViewportClient = static_cast<FEditorViewportClient*>(GEditor->GetActiveViewport()->GetClient());
        return EditorViewportClient->GetViewLocation();
    }
#endif    

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->PlayerCameraManager)
    {
        APawn* ControlledPawn = PlayerController->GetPawn();
        if (ControlledPawn)
        {
            UCameraComponent* CameraComponent = ControlledPawn->FindComponentByClass<UCameraComponent>();
            if (CameraComponent)
            {
                return CameraComponent->GetComponentLocation();
            }
        }
    }

    return FVector::ZeroVector;
}

void AShowActionMakerGameMode::UpdateCameraPathPoint(FCameraPathPoint* CameraPathPoint)
{
	if (OnUpdateCameraView.IsBound())
	{
		OnUpdateCameraView.Execute(Caster, CameraPathPoint, { DebugCameraHelper });
	}
}
