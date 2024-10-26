// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
#include "ActionServerExecutor.h"

// Sets default values for this component's properties
UActionComponent::UActionComponent() : Super()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Dispose();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (TMap<FName, UActionBase*>::TIterator It = ActiveActions.CreateIterator(); It; ++It)
	{
		UActionBase* ActionBasePtr = It.Value();

		if (ActionBasePtr)
		{
			// 액션 완료 여부 확인
			if (ActionBasePtr->IsCompleted())
			{
				if (!ActionBasePtr->IsDontDestroy())
				{
					UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
					PoolManager->ReturnPooledObject(ActionBasePtr, ActionBasePtr->GetObjectPoolType());

					It.RemoveCurrent();
				}
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

bool UActionComponent::DefaultFilterRule(const FActionBaseData* ActionBaseData)
{
	checkf(ActionBaseData != nullptr, TEXT("UActionComponent::DefaultFilterRule ActionBaseData is nullptr"));

	if (!MainActionPtr)
	{
		return true;
	}

	if (MainActionPtr->GetState() == EActionState::Exec)
	{
		return false;
	}

	if (MainActionPtr->GetActionBaseData()->Priority > ActionBaseData->Priority)
	{
		return false;
	}

	return true;
}

void UActionComponent::Dispose()
{
	MainActionPtr = nullptr;

	if (ActiveActions.Num() > 0)
	{
		UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();

		TArray<UActionBase*> Values;
		ActiveActions.GenerateValueArray(Values);
		for (UActionBase* Value : Values)
		{
			PoolManager->ReturnPooledObject(Value, Value->GetObjectPoolType());
		}
		ActiveActions.Empty();
	}
}

bool UActionComponent::CanUseAction(const FActionBaseData* ActionBaseData, ActionFilterFuncPtr ActionFilter)
{
	checkf(ActionBaseData != nullptr, TEXT("UActionComponent::CanUseActionFromPool ActionBaseData is nullptr"));

	if (ActionFilter)
	{
		if (!ActionFilter(ActionBaseData))
		{
			return false;
		}
	}
	else if (!DefaultFilterRule(ActionBaseData))
	{
		return false;
	}

	return true;
}

bool UActionComponent::CanUseAction(UActionBase* ActionBasePtr, ActionFilterFuncPtr ActionFilter)
{
	checkf(ActionBasePtr != nullptr, TEXT("UActionComponent::CanUseActionFromPool ActionBasePtr is nullptr"));
	return CanUseAction(ActionBasePtr->GetActionBaseData(), ActionFilter);
}

void UActionComponent::CancelAction(const FName& ActionName)
{
	UActionBase** ActionBasePtrAddress = ActiveActions.Find(ActionName);
	if (!ActionBasePtrAddress)
	{
		UE_LOG(LogTemp, Error, TEXT("UActionComponent::CancelAction ActionBase is nullptr [ %s ]"), *ActionName.ToString());
		return;
	}

	UActionBase* ActionPtr = *ActionBasePtrAddress;
	ActionPtr->Cancel();

	if (ActionPtr == MainActionPtr)
	{
		MainActionPtr = nullptr;
	}

	UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
	PoolManager->ReturnPooledObject(ActionPtr, ActionPtr->GetObjectPoolType());

	ActiveActions.Remove(ActionName);
}

void UActionComponent::CancelAction(UActionBase* ActionPtr)
{
	checkf(ActionPtr != nullptr, TEXT("UActionComponent::CancelAction ActionPtr is nullptr"));

	FName ActionName = ActionPtr->GetActionName();
	CancelAction(ActionName);
}

void UActionComponent::CancelMainAction()
{
	if (!MainActionPtr)
	{
		return;
	}

	CancelAction(MainActionPtr);
}

//void UActionComponent::DoAction(UActionBase* ActionBasePtr)
//{
//	checkf(ActionBasePtr != nullptr, TEXT("UActionComponent::DoAction ActionBasePtr is nullptr"));
//
//	if (MainActionPtr && MainActionPtr != ActionBasePtr)
//	{
//		MainActionPtr->Cancel();
//	}
//	else
//	{
//		FName ActionName = ActionBasePtr->GetActionName();
//		if (!ActiveActions.Contains(ActionName))
//		{
//			ActiveActions.Add(ActionName, ActionBasePtr);
//		}
//	}
//
//	ActionBasePtr->Casting();
//	MainActionPtr = ActionBasePtr;
//}
