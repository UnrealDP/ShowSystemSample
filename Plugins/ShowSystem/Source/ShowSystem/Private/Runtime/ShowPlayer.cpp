// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowPlayer.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowSequencerComponent.h"

/** Implement this for initialization of instances of the system */
void UShowPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
    if (Initialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("UShowPlayer::Initialize called again, skipping as it is already initialized."));
        return;
    }
    ON_SCOPE_EXIT
    {
        Initialized = true;
    };

    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("UShowPlayer Initialized successfully."));
}

/** Implement this for deinitialization of instances of the system */
void UShowPlayer::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("UShowPlayer: Object pool cleaned up."));
}

void UShowPlayer::Tick(float DeltaTime)
{
    if (!Initialized)
    {
        return;
    }
}

void UShowPlayer::PlaySoloShow(AActor* Owner, UShowSequencer* ShowSequencer)
{
    checkf(Owner, TEXT("UShowPlayer::PlaySoloShow: The Owner provided is invalid or null."));
    checkf(ShowSequencer, TEXT("UShowPlayer::PlaySoloShow: The ShowSequencer provided is invalid or null."));

    UShowSequencerComponent* ShowSequencerComponent = Owner->FindComponentByClass<UShowSequencerComponent>();
    if (!ShowSequencerComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UShowPlayer::PlaySoloShow add UShowSequencerComponent"));

        ShowSequencerComponent = NewObject<UShowSequencerComponent>(Owner);
        if (ShowSequencerComponent)
        {
            Owner->AddInstanceComponent(ShowSequencerComponent);
            ShowSequencerComponent->RegisterComponent();
        }
    }

    ShowSequencerComponent->PlayShow(ShowSequencer);
}

void UShowPlayer::StopSoloShow(AActor* Owner, int32 ID)
{
    checkf(Owner, TEXT("UShowPlayer::StopSoloShow: The Owner provided is invalid or null."));

    UShowSequencerComponent* ShowSequencerComponent = Owner->FindComponentByClass<UShowSequencerComponent>();
    checkf(ShowSequencerComponent, TEXT("UShowPlayer::StopSoloShow: The ShowSequencerComponent provided is invalid or null."));

    ShowSequencerComponent->StopShow(ID);
}