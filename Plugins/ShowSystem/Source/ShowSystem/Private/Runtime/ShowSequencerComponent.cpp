// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencerComponent.h"
#include "RunTime/ShowSequencer.h"

// Sets default values for this component's properties
UShowSequencerComponent::UShowSequencerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UShowSequencerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UShowSequencerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TObjectPtr<UShowSequencer> showSequencer : ShowSequencers)
	{
		showSequencer->Tick(DeltaTime);
	}
}

void UShowSequencerComponent::PlayShow(UShowSequencer* ShowSequencer)
{
	checkf(ShowSequencer, TEXT("UShowSequencerComponent::PlayShow: The OShowSequencer provided is invalid or null."));

	int32 ID = ShowSequencers.Add(ShowSequencer);
	ShowSequencer->SetOwner(GetOwner());
	ShowSequencer->SetID(ID);
}

void UShowSequencerComponent::StopShow(UShowSequencer* ShowSequencer)
{
	checkf(ShowSequencer, TEXT("UShowSequencerComponent::PlayShow: The OShowSequencer provided is invalid or null."));

	ShowSequencer->ClearID();
	ShowSequencer->ClearOwner();
	ShowSequencers.RemoveAt(ShowSequencer->GetID());
}

void UShowSequencerComponent::PauseShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::UnPauseShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::ChangeSpeedShow(UShowSequencer* ShowSequencer, float Speed)
{}