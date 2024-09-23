// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowPlayer.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowSequencerComponent.h"

bool UShowPlayer::ShouldCreateSubsystem(UObject* Outer) const { return true; }

/** Implement this for initialization of instances of the system */
void UShowPlayer::Initialize(FSubsystemCollectionBase& Collection)
{
    Initialized = false;
    ON_SCOPE_EXIT
    {
        Initialized = true;
    };

    UGameInstanceSubsystem::Initialize(Collection);
}

/** Implement this for deinitialization of instances of the system */
void UShowPlayer::Deinitialize()
{
    UGameInstanceSubsystem::Deinitialize();
}

void UShowPlayer::Tick(float DeltaTime)
{
    if (!Initialized)
    {
        return;
    }

    double T = FApp::GetCurrentTime();
    UE_LOG(LogTemp, Warning, TEXT("Tick running with current time: %f, delta time: %f, frame: %d"),
        T, DeltaTime, GFrameCounter);
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
            ShowSequencerComponent->RegisterComponent();
            Owner->AddInstanceComponent(ShowSequencerComponent);
        }
    }

    ShowSequencerComponent->PlayShow(ShowSequencer);
}

void UShowPlayer::StopSoloShow(AActor* Owner, UShowSequencer* ShowSequencer)
{
    checkf(Owner, TEXT("UShowPlayer::StopSoloShow: The Owner provided is invalid or null."));
    checkf(ShowSequencer, TEXT("UShowPlayer::StopSoloShow: The ShowSequencer provided is invalid or null."));

    UShowSequencerComponent* ShowSequencerComponent = Owner->FindComponentByClass<UShowSequencerComponent>();
    checkf(ShowSequencer, TEXT("UShowPlayer::StopSoloShow: The ShowSequencerComponent provided is invalid or null."));

    ShowSequencerComponent->StopShow(ShowSequencer);
}