// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"


// Sets default values for this component's properties
UActionComponent::UActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MainActionPtr = nullptr;

	UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();

	for (auto& Pair : ActionPool)
	{
		UActionBase* Value = Pair.Value;
		PoolManager->ReturnPooledObject(Value, Value->GetObjectPoolType());
		Pair.Value = nullptr;
	}
	ActionPool.Empty();

	for (auto& Pair : OneShotActions)
	{
		UActionBase* Value = Pair.Value;
		PoolManager->ReturnPooledObject(Value, Value->GetObjectPoolType());
		Pair.Value = nullptr;
	}
	OneShotActions.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<UActionBase*> Values;
	ActionPool.GenerateValueArray(Values);
	for (UActionBase* Value : Values)
	{
		if (!Value)
		{
			continue;
		}

		Value->Tick(DeltaTime);
	}

	for (TMap<FName, UActionBase*>::TIterator It = OneShotActions.CreateIterator(); It; ++It)
	{
		UActionBase* ActionBasePtr = It.Value();

		if (ActionBasePtr)
		{
			// 액션 완료 여부 확인
			if (ActionBasePtr->IsCompleted())
			{
				UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
				PoolManager->ReturnPooledObject(ActionBasePtr, ActionBasePtr->GetObjectPoolType());
				ActionBasePtr = nullptr;

				It.RemoveCurrent();
			}
			else
			{
				ActionBasePtr->Tick(DeltaTime);
			}
		}
		else
		{
			checkf(false, TEXT("UActionComponent::TickComponent ActionBase is nullptr"));
			It.RemoveCurrent();
		}
	}

	if (MainActionPtr)
	{
		// Cast 나 Exec 상태가 아니면 몸을 점유하는 메인 액션에서는 뺀다
		if (MainActionPtr->GetState() != EActionState::Cast &&
			MainActionPtr->GetState() != EActionState::Exec)
		{
			MainActionPtr = nullptr;
		}
	}
}

bool UActionComponent::DefaultFilterRule(const FName& ActionName, const FActionBaseData* InActionBaseData)
{
	if (!MainActionPtr)
	{
		return true;
	}

	if (MainActionPtr->GetState() == EActionState::Exec)
	{
		return false;
	}

	if (MainActionPtr->GetActionBaseData()->Priority > InActionBaseData->Priority)
	{
		return false;
	}

	return true;
}

void UActionComponent::ClearActionPool()
{
	if (ActionPool.Num() == 0)
	{
		return;
	}

	UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();

	TArray<UActionBase*> Values;
	ActionPool.GenerateValueArray(Values);
	for (UActionBase* Value : Values)
	{
		PoolManager->ReturnPooledObject(Value, Value->GetObjectPoolType());
	}
	ActionPool.Empty();
}

UActionBase* UActionComponent::DoActionPool(const FName& ActionName, ActionFilterFuncPtr ActionFilter)
{
	UActionBase** ActionBasePtrAddress = ActionPool.Find(ActionName);
	if (!ActionBasePtrAddress)
	{
		UE_LOG(LogTemp, Error, TEXT("UActionComponent::DoActionPool ActionBase is nullptr [ %s ]"), *ActionName.ToString());
		return nullptr;
	}

	UActionBase* ActionBasePtr = *ActionBasePtrAddress;
	if (ActionFilter)
	{
		if (!ActionFilter(this, ActionName, ActionBasePtr->GetActionBaseData()))
		{
			return nullptr;
		}
	}
	else if (!DefaultFilterRule(ActionName, ActionBasePtr->GetActionBaseData()))
	{
		return nullptr;
	}

	if(MainActionPtr)
	{
		MainActionPtr->Cancel();
	}

	MainActionPtr = ActionBasePtr;
	return ActionBasePtr;
}

