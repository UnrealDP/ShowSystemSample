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

//class UActionBase;
class UActionComponent;
//struct FActionBaseData;
//struct FActionBaseShowData;

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
	UActionBase* DoAction(const FName& ActionName, ActionFilterFuncPtr ActionFilter = nullptr);

	const UActionBase* GetMainAction() const { return MainAction; }

	template<typename TActionData, typename TActionShowData, typename TActionObject>
	void InitializeActionPool(const TArray<FName>& ActionNames)
	{
		static_assert(TIsDerivedFrom<TActionData, FActionBaseData>::IsDerived, "TActionData must be derived from FActionBaseData");
		static_assert(TIsDerivedFrom<TActionShowData, FActionBaseShowData>::IsDerived, "TActionShowData must be derived from FActionShowBaseData");
		static_assert(TIsDerivedFrom<TActionObject, UActionBase>::IsDerived, "TActionObject must be derived from UActionBase");

		ClearActionPool();

		UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
		for (FName ActionName : ActionNames)
		{
			TActionData* ActionData = DataTableManager::Data<TActionData>(EDataTable::SkillData, ActionName);
			checkf(ActionData != nullptr, TEXT("UActionComponent::InitializeActionPool SkillData Fail [ %s ]"), *ActionName.ToString());

			TActionShowData* SkillShowData = DataTableManager::Data<TActionShowData>(EDataTable::SkillShowData, ActionName);

			TActionObject* ActionBase = PoolManager->GetPooledObject<TActionObject>(EObjectPoolType::ObjectPool_Action);
			checkf(ActionBase != nullptr, TEXT("UActionComponent::InitializeActionPool GetPooledObject Fail [ %s ]"), *ActionName.ToString());

			if (ActionBase)
			{
				ActionBase->Initialize(GetOwner(), ActionName, ActionData, SkillShowData);
				ActionPool.Add(ActionName, ActionBase);
			}
		}
	}

private:
	UActionBase* MainAction = nullptr;

	TMap<FName, UActionBase*> ActionPool;

	TMap<FName, UActionBase*> OneShotActions;
};
