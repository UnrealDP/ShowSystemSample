// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataTableManager.h"
#include "Data/ActionBaseData.h"
#include "Data/ActionBaseShowData.h"
#include "ActionBase.h"
#include "ObjectPool/ObjectPoolManager.h"
#include "ActionComponent.generated.h"

class UActionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOWACTIONSYSTEM_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
	virtual bool DefaultFilterRule(const FName&, const FActionBaseData*);

public:	
	// 외부로직으로 액션을 사용할지 여부를 결정하는 필터
	typedef bool (*ActionFilterFuncPtr)(const UActionComponent*, const FName&, const FActionBaseData*);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ClearActionPool();

	UActionBase* DoActionPool(const FName& ActionName, ActionFilterFuncPtr ActionFilter = nullptr);

	const UActionBase* GetMainActionPtr() const { return MainActionPtr; }
	UActionBase* GetAction(const FName& ActionName) const { return ActionPool.FindRef(ActionName); }

	template<typename TActionObject, typename TActionData, typename TActionShowData>
	void InitializeActionPool(const TArray<FName>& ActionNames)
	{
		static_assert(TIsDerivedFrom<TActionObject, UActionBase>::IsDerived, "TActionObject must be derived from UActionBase");
		static_assert(TIsDerivedFrom<TActionData, FActionBaseData>::IsDerived, "TActionData must be derived from FActionBaseData");
		static_assert(TIsDerivedFrom<TActionShowData, FActionBaseShowData>::IsDerived, "TActionShowData must be derived from FActionShowBaseData");		

		ClearActionPool();

		UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
		for (FName ActionName : ActionNames)
		{
			TActionData* ActionData = DataTableManager::Data<TActionData>(ActionName);
			checkf(ActionData, TEXT("UActionComponent::InitializeActionPool SkillData Fail [ %s ]"), *ActionName.ToString());

			TActionShowData* SkillShowData = DataTableManager::Data<TActionShowData>(ActionName);

			UActionBase* ActionBasePtr = static_cast<UActionBase*>(PoolManager->GetPooledObject<TActionObject>());
			checkf(ActionBasePtr, TEXT("UActionComponent::InitializeActionPool GetPooledObject Fail [ %s ]"), *ActionName.ToString());

			if (ActionBasePtr)
			{
				ActionBasePtr->Initialize<TActionObject, TActionData, TActionShowData>(GetOwner(), ActionName, ActionData, SkillShowData);
				ActionPool.Add(ActionName, ActionBasePtr);
			}
		}
	}

	// 서버 연동하게 되면 서버에서 액션 패킷을 보내주면 해야해서 로직 바껴야함
	// 지금은 클라 베이스로 우선 만들어서 로직 검증부터 한다
	template<typename TActionObject, typename TActionData, typename TActionShowData>
	UActionBase* DoAction(const FName& ActionName, ActionFilterFuncPtr ActionFilter)
	{
		UActionBase** ActionBasePtrAddress = OneShotActions.Find(ActionName);
		UActionBase* ActionBasePtr = nullptr;
		if (ActionBasePtrAddress)
		{
			ActionBasePtr = *ActionBasePtrAddress;

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

			ActionBasePtr->Cancel();
		}
		else
		{
			TActionData* ActionBaseData = DataTableManager::Data<TActionData>(ActionName);
			checkf(ActionBaseData != nullptr, TEXT("UActionComponent::DoAction ActionBaseData Fail"));

			TActionShowData* ActionBaseShowData = DataTableManager::Data<TActionShowData>(ActionName);

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
			ActionBasePtr = static_cast<UActionBase*>(PoolManager->GetPooledObject<TActionObject>());
			checkf(ActionBasePtr != nullptr, TEXT("UActionComponent::DoAction GetPooledObject Fail"));

			ActionBasePtr->Initialize<TActionObject, TActionData, TActionShowData>(GetOwner(), ActionName, ActionBaseData, ActionBaseShowData);
			OneShotActions.Add(ActionName, ActionBasePtr);
		}

		if (ActionBasePtr)
		{
			if (MainActionPtr)
			{
				MainActionPtr->Cancel();
			}

			MainActionPtr = ActionBasePtr;
		}

		return ActionBasePtr;
	}

private:
	UActionBase* MainActionPtr = nullptr;

	TMap<FName, UActionBase*> ActionPool;

	TMap<FName, UActionBase*> OneShotActions;
};
