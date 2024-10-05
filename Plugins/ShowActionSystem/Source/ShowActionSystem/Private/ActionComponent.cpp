// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionComponent.h"
//#include "DataTableManager.h"
#include "Data/SkillData.h"
#include "Data/SkillShowData.h"
//#include "ActionBase.h"
//#include "ObjectPool/ObjectPoolManager.h"


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
	MainAction = nullptr;

	UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();

	for (auto& Pair : ActionPool)
	{
		UActionBase* Value = Pair.Value;
		PoolManager->ReturnPooledObject(Value, EObjectPoolType::ObjectPool_Action);
		Pair.Value = nullptr;
	}
	ActionPool.Empty();

	for (auto& Pair : OneShotActions)
	{
		UActionBase* Value = Pair.Value;
		PoolManager->ReturnPooledObject(Value, EObjectPoolType::ObjectPool_Action);
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
		UActionBase* ActionBase = It.Value();

		if (ActionBase)
		{
			// 액션 완료 여부 확인
			if (ActionBase->IsCompleted())
			{
				UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
				PoolManager->ReturnPooledObject(ActionBase, EObjectPoolType::ObjectPool_Action);
				ActionBase = nullptr;

				It.RemoveCurrent();
			}
			else
			{
				ActionBase->Tick(DeltaTime);
			}
		}
		else
		{
			checkf(false, TEXT("UActionComponent::TickComponent ActionBase is nullptr"));
			It.RemoveCurrent();
		}
	}

	if (MainAction)
	{
		// Cast 나 Exec 상태가 아니면 몸을 점유하는 메인 액션에서는 뺀다
		if (MainAction->GetState() != EActionState::Cast &&
			MainAction->GetState() != EActionState::Exec)
		{
			MainAction = nullptr;
		}
	}
}

bool UActionComponent::DefaultFilterRule(const FName& ActionName, const FActionBaseData* InActionBaseData)
{
	if (!MainAction)
	{
		return true;
	}

	if (MainAction->GetState() == EActionState::Exec)
	{
		return false;
	}

	if (MainAction->GetActionBaseData()->Priority > InActionBaseData->Priority)
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
		PoolManager->ReturnPooledObject(Value, EObjectPoolType::ObjectPool_Action);
	}
	ActionPool.Empty();
}

UActionBase* UActionComponent::DoActionPool(const FName& ActionName, ActionFilterFuncPtr ActionFilter)
{
	UActionBase** ActionBasePtr = ActionPool.Find(ActionName);
	if (!ActionBasePtr)
	{
		UE_LOG(LogTemp, Error, TEXT("UActionComponent::DoActionPool ActionBase is nullptr [ %s ]"), *ActionName.ToString());
		return nullptr;
	}

	UActionBase* ActionBase = *ActionBasePtr;
	if (ActionFilter)
	{
		if (!ActionFilter(this, ActionName, ActionBase->GetActionBaseData()))
		{
			return nullptr;
		}
	}
	else if (!DefaultFilterRule(ActionName, ActionBase->GetActionBaseData()))
	{
		return nullptr;
	}

	if(MainAction)
	{
		MainAction->Cancel();
	}

	MainAction = ActionBase;
	return ActionBase;
}

// 서버 연동하게 되면 서버에서 액션 패킷을 보내주면 해야해서 로직 바껴야함
// 지금은 클라 베이스로 우선 만들어서 로직 검증부터 한다
UActionBase* UActionComponent::DoAction(const FName& ActionName, ActionFilterFuncPtr ActionFilter)
{
	UActionBase** ActionBasePtr = OneShotActions.Find(ActionName);
	UActionBase* ActionBase = nullptr;
	if (ActionBasePtr)
	{
		ActionBase = *ActionBasePtr;

		if (ActionFilter)
		{
			if (!ActionFilter(this, ActionName, ActionBase->GetActionBaseData()))
			{
				return nullptr;
			}
		}
		else if (!DefaultFilterRule(ActionName, ActionBase->GetActionBaseData()))
		{
			return nullptr;
		}

		ActionBase->Cancel();
	}
	else
	{
		FSkillData* ActionBaseData = DataTableManager::Data<FSkillData>(ActionName);
		checkf(ActionBaseData != nullptr, TEXT("UActionComponent::DoAction ActionBaseData Fail"));

		FSkillShowData* ActionBaseShowData = DataTableManager::Data<FSkillShowData>(ActionName);

		if (ActionFilter)
		{
			if (!ActionFilter(this, ActionName, ActionBaseData))
			{
				return nullptr;
			}
		}
		else if (!DefaultFilterRule(ActionName, ActionBaseData))
		{
			return nullptr;
		}

		UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
		ActionBase = PoolManager->GetPooledObject<UActionBase>(EObjectPoolType::ObjectPool_Action);
		checkf(ActionBase != nullptr, TEXT("UActionComponent::DoAction GetPooledObject Fail"));

		ActionBase->Initialize(GetOwner(), ActionName, ActionBaseData, ActionBaseShowData);
		OneShotActions.Add(ActionName, ActionBase);
	}

	if (ActionBase)
	{
		if (MainAction)
		{
			MainAction->Cancel();
		}

		MainAction = ActionBase;
	}

	return ActionBase;
}
