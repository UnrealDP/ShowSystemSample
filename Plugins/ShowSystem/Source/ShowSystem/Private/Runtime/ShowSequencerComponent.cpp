// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowSequencerComponent.h"
#include "RunTime/ShowSequencer.h"
#include "RunTime/ShowSystem.h"

// Sets default values for this component's properties
UShowSequencerComponent::UShowSequencerComponent() : Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UShowSequencerComponent::Initialize()
{
	AActor* Owner = GetOwner();
	checkf(Owner, TEXT("UShowSequencerComponent::BeginPlay: The Owner is invalid."));

	UWorld* World = Owner->GetWorld();
	checkf(World, TEXT("UShowSequencerComponent::BeginPlay: The World is invalid."));

	PoolManager = World->GetSubsystem<UObjectPoolManager>();
}

void UShowSequencerComponent::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}

void UShowSequencerComponent::BeginDestroy()
{
	if (HasBegunPlay())
	{
		checkf(PoolManager, TEXT("UShowSequencerComponent::NewShowSequencer: The PoolManager is invalid."));

		for (UShowSequencer* ShowSequencerPtr : ShowSequencers)
		{
			PoolManager->ReturnPooledObject(ShowSequencerPtr, EObjectPoolType::ObjectPool_ShowSequencer);
		}
		ShowSequencers.Empty();
	}

	Super::BeginDestroy();
}

// Called every frame
void UShowSequencerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (UShowSequencer* ShowSequencerPtr : ShowSequencers)
	{
		ShowSequencerPtr->Tick(DeltaTime);
	}
}

UShowSequencer* UShowSequencerComponent::NewShowSequencer(const FSoftObjectPath& ShowPath)
{
	AActor* Owner = GetOwner();
	checkf(Owner, TEXT("UShowSequencerComponent::NewShowSequencer: The Owner is invalid."));
	checkf(PoolManager, TEXT("UShowSequencerComponent::NewShowSequencer: The PoolManager is invalid."));

	if (UShowSequenceAsset* LoadedShowSequenceAsset = Cast<UShowSequenceAsset>(ShowPath.TryLoad())) 
	{
		UShowSequencer* NewShowSequencer = PoolManager->GetPooledObject<UShowSequencer>();
		NewShowSequencer->Initialize(Owner, LoadedShowSequenceAsset);
		ShowSequencers.Add(NewShowSequencer);
		return NewShowSequencer;
	}
	return nullptr;
}

void UShowSequencerComponent::PlayShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::PlayShow: The OShowSequencer provided is invalid or null."));

	InShowSequencer->Play();
}

void UShowSequencerComponent::ResetShow(UShowSequencer* InShowSequencer) const
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::StopShow: InShowSequencer is invalid or null"));

	InShowSequencer->Reset();
}

// 자주 사용되는 Show 같은 경우에는 DontDestroy를 사용하여 DisposeShow를 사용하지 않는 것이 좋음
void UShowSequencerComponent::DisposeShow(UShowSequencer* InShowSequencer)
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::DisposeShow: InShowSequencer is invalid or null"));
	checkf(PoolManager, TEXT("UShowSequencerComponent::NewShowSequencer: The PoolManager is invalid."));

	ShowSequencers.Remove(InShowSequencer);
	PoolManager->ReturnPooledObject(InShowSequencer, EObjectPoolType::ObjectPool_ShowSequencer);
}

void UShowSequencerComponent::PauseShow(UShowSequencer* InShowSequencer) const
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::PauseShow: InShowSequencer is invalid or null."));

	InShowSequencer->Pause();
}

void UShowSequencerComponent::UnPauseShow(UShowSequencer* InShowSequencer) const
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::UnPauseShow: InShowSequencer is invalid or null"));

	InShowSequencer->UnPause();
}

void UShowSequencerComponent::ChangeShowTimeScalse(UShowSequencer* InShowSequencer, float InTimeScalse) const
{
	checkf(InShowSequencer, TEXT("UShowSequencerComponent::ChangeShowTimeScalse: InShowSequencer is invalid or null"));

	InShowSequencer->ChangeTimeScale(InTimeScalse);
}