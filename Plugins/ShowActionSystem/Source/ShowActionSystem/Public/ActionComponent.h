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
struct FActionBaseData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOWACTIONSYSTEM_API UActionComponent : public UActorComponent
{
	GENERATED_BODY()

#if WITH_EDITOR
	friend class AShowActionMakerGameMode;
#endif

public:	
	// Sets default values for this component's properties
	UActionComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
	virtual bool DefaultFilterRule(const FActionBaseData* ActionBaseData);

public:	
	// setter getter
	const UActionBase* GetMainActionPtr() const { return MainActionPtr; }
	UActionBase* GetActiveAction(const FName& ActionName) const { return ActiveActions.FindRef(ActionName); }
	// end of setter getter
	
	// 외부로직으로 액션을 사용할지 여부를 결정하는 필터
	typedef bool (*ActionFilterFuncPtr)(const FActionBaseData* ActionBaseData);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Dispose();

	template<typename TActionData, typename std::enable_if<std::is_base_of<FActionBaseData, TActionData>::value, int>::type = 0>
	bool CanUseAction(const FName& ActionName, ActionFilterFuncPtr ActionFilter)
	{
		TActionData* ActionData = DataTableManager::Data<TActionData>(ActionName);
		checkf(ActionData != nullptr, TEXT("UActionComponent::CanUseAction ActionData is nullptr"));

		return CanUseAction(ActionData, ActionFilter);
	}
	bool CanUseAction(const FActionBaseData* ActionBaseData, ActionFilterFuncPtr ActionFilter);
	bool CanUseAction(UActionBase* ActionBasePtr, ActionFilterFuncPtr ActionFilter);

	void CancelAction(const FName& ActionName);
	void CancelAction(UActionBase* ActionPtr);
	void CancelMainAction();

	template<typename TActionObject, typename TActionData, typename TActionShowData>
	UActionBase* NewAction(FName& ActionName)
	{
		static_assert(TIsDerivedFrom<TActionObject, UActionBase>::IsDerived, "TActionObject must be derived from UActionBase");
		static_assert(TIsDerivedFrom<TActionData, FActionBaseData>::IsDerived, "TActionData must be derived from FActionBaseData");
		static_assert(TIsDerivedFrom<TActionShowData, FActionBaseShowData>::IsDerived, "TActionShowData must be derived from FActionShowBaseData");

		UObjectPoolManager* PoolManager = GetOwner()->GetWorld()->GetSubsystem<UObjectPoolManager>();
		TActionData* ActionData = DataTableManager::Data<TActionData>(ActionName);
		checkf(ActionData, TEXT("UActionComponent::InitializeActionPool SkillData Fail [ %s ]"), *ActionName.ToString());

		TActionShowData* SkillShowData = DataTableManager::Data<TActionShowData>(ActionName);

		UActionBase* ActionBasePtr = static_cast<UActionBase*>(PoolManager->GetPooledObject<TActionObject>());
		checkf(ActionBasePtr, TEXT("UActionComponent::InitializeActionPool GetPooledObject Fail [ %s ]"), *ActionName.ToString());

		ActionBasePtr->Initialize<TActionObject, TActionData, TActionShowData>(GetOwner(), ActionName, ActionData, SkillShowData);
		ActiveActions.Add(ActionName, ActionBasePtr);

		return ActionBasePtr;
	}

	// TODO: (DIPI) 서버 연동하게 되면 서버에서 액션 패킷을 보내주면 해야해서 로직 바껴야함
	// 지금은 클라 베이스로 우선 만들어서 로직 검증부터 한다
	/*template<typename TActionObject, typename TActionData, typename TActionShowData>
	UActionBase* DoAction(const FName& ActionName)
	{
		UActionBase** ActionBasePtrAddress = ActiveActions.Find(ActionName);
		UActionBase* ActionBasePtr = nullptr;
		if (ActionBasePtrAddress)
		{
			ActionBasePtr = *ActionBasePtrAddress;
			if (ActionBasePtr == MainActionPtr)
			{
				MainActionPtr = nullptr;
			}
			ActionBasePtr->Cancel();
		}
		else
		{
			ActionBasePtr = NewAction<TActionObject, TActionData, TActionShowData>(ActionName);
			checkf(ActionBasePtr != nullptr, TEXT("UActionComponent::DoAction GetPooledObject Fail"));
		}

		DoAction(ActionBasePtr);
		return ActionBasePtr;
	}
	void DoAction(UActionBase* ActionBasePtr);*/

private:
	UActionBase* MainActionPtr = nullptr;
	TMap<FName, UActionBase*> ActiveActions;
};
