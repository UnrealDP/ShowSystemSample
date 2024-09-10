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

	// ...
}

void UShowSequencerComponent::PlayShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::StopShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::PauseShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::UnPauseShow(UShowSequencer* ShowSequencer)
{}

void UShowSequencerComponent::ChangeSpeedShow(UShowSequencer* ShowSequencer, float Speed)
{}