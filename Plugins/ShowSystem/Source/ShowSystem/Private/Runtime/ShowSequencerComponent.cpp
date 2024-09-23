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

int32 UShowSequencerComponent::PlayShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::PlayShow: The OShowSequencer provided is invalid or null."));

	int32 ID = ShowSequencers.Add(InShowSequencer);
	InShowSequencer->Initialize(ID, GetOwner());
	InShowSequencer->Play();
	return ID;
}

void UShowSequencerComponent::StopShow(int32 ID)
{
	checkf(ShowSequencers.IsValidIndex(ID), TEXT("UShowSequencerComponent::StopShow: ID is invalid."), ID);

	UShowSequencer* FoundShowSequencer = ShowSequencers[ID];
	checkf(FoundShowSequencer, TEXT("UShowSequencerComponent::StopShow: FoundShowSequencer is invalid or null. [ %d ]"), ID);

	FoundShowSequencer->Stop();
	FoundShowSequencer->Dispose();
}

void UShowSequencerComponent::DisposeShow(int32 ID)
{
	checkf(ShowSequencers.IsValidIndex(ID), TEXT("UShowSequencerComponent::DisposeShow: ID is invalid."), ID);

	UShowSequencer* FoundShowSequencer = ShowSequencers[ID];
	checkf(FoundShowSequencer, TEXT("UShowSequencerComponent::DisposeShow: FoundShowSequencer is invalid or null. [ %d ]"), ID);

	FoundShowSequencer->Dispose();
	ShowSequencers.RemoveAt(ID);
}

void UShowSequencerComponent::PauseShow(int32 ID)
{
	checkf(ShowSequencers.IsValidIndex(ID), TEXT("UShowSequencerComponent::PauseShow: ID is invalid."), ID);
	
	UShowSequencer* FoundShowSequencer = ShowSequencers[ID];
	checkf(FoundShowSequencer, TEXT("UShowSequencerComponent::PauseShow: FoundShowSequencer is invalid or null. [ %d ]"), ID);

	FoundShowSequencer->Pause();
}

void UShowSequencerComponent::UnPauseShow(int32 ID)
{
	checkf(ShowSequencers.IsValidIndex(ID), TEXT("UShowSequencerComponent::UnPauseShow: ID is invalid."), ID);

	UShowSequencer* FoundShowSequencer = ShowSequencers[ID];
	checkf(FoundShowSequencer, TEXT("UShowSequencerComponent::UnPauseShow: FoundShowSequencer is invalid or null. [ %d ]"), ID);

	FoundShowSequencer->UnPause();
}

void UShowSequencerComponent::ChangeSpeedShow(int32 ID, float Speed)
{
	checkf(ShowSequencers.IsValidIndex(ID), TEXT("UShowSequencerComponent::ChangeSpeedShow: ID is invalid."), ID);

	UShowSequencer* FoundShowSequencer = ShowSequencers[ID];
	checkf(FoundShowSequencer, TEXT("UShowSequencerComponent::ChangeSpeedShow: FoundShowSequencer is invalid or null. [ %d ]"), ID);

	FoundShowSequencer->ChangeSpeed(Speed);
}