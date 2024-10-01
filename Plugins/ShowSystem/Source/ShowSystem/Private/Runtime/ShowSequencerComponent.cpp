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

void UShowSequencerComponent::BeginDestroy()
{
	ShowSequencers.Empty();
	Super::BeginDestroy();
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

void UShowSequencerComponent::PlayShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::PlayShow: The OShowSequencer provided is invalid or null."));

	ShowSequencers.Add(InShowSequencer);
	InShowSequencer->Initialize(GetOwner());
	InShowSequencer->Play();
}

void UShowSequencerComponent::StopShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::StopShow: InShowSequencer is invalid or null"));

	InShowSequencer->Stop();
	// InShowSequencer->Dispose(); 를 안하는 이유는 Stop은 다시 재생할 수도 있음
	// 완전히 삭제하고 싶으면 DisposeShow를 사용해야함
	// 특히나 자주 사용되는 Show 같은 경우에는 DontDestroy를 사용하여 DisposeShow를 사용하지 않는 것이 좋음
	InShowSequencer->ClearOwner();
}

void UShowSequencerComponent::DisposeShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::DisposeShow: InShowSequencer is invalid or null"));

	InShowSequencer->Dispose();
	ShowSequencers.Remove(InShowSequencer);
}

void UShowSequencerComponent::PauseShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::PauseShow: InShowSequencer is invalid or null."));

	InShowSequencer->Pause();
}

void UShowSequencerComponent::UnPauseShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::UnPauseShow: InShowSequencer is invalid or null"));

	InShowSequencer->UnPause();
}

void UShowSequencerComponent::ChangeSpeedShow(UShowSequencer* InShowSequencer, float Speed)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::ChangeSpeedShow: InShowSequencer is invalid or null"));

	InShowSequencer->ChangeSpeed(Speed);
}