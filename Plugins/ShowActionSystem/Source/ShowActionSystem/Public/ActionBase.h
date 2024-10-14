// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectPool/Pooled.h"
#include "Data/ActionBaseData.h"
#include "ObjectPool/ObjectPoolType.h"
#include "EDataTable.h"
#include "ActionBase.generated.h"

class UShowSequencer;

/**
 * 액션의 기본 기능을 정의한 클래스
 * 하나의 액션을 정의함
 * 액션을 진행하는 서버 시뮬레이션 로직은 ActionExecutor 에 구현
 * 데디케이트 서버를 사용한다면 ActionExecutor 로 가능할지 구조를 다시 고려해야함
 */
UCLASS(Abstract)
class SHOWACTIONSYSTEM_API UActionBase : public UObject, public IPooled
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	template<typename TActionObject, typename TActionData, typename TActionShowData>
	TActionObject* Initialize(TObjectPtr<AActor> InOwner, const FName& InActionName, const TActionData* InActionBaseData, const TActionShowData* InActionBaseShowData)
	{
		static_assert(TIsDerivedFrom<TActionObject, UActionBase>::IsDerived, "TActionObject must be derived from UActionBase");
		static_assert(TIsDerivedFrom<TActionData, FActionBaseData>::IsDerived, "TActionData must be derived from FActionBaseData");
		static_assert(TIsDerivedFrom<TActionShowData, FActionBaseShowData>::IsDerived, "TActionShowData must be derived from FActionBaseShowData");

		checkf(InOwner != nullptr, TEXT("UActionBase::Initialize InOwner is invalid"));
		checkf(InActionBaseData != nullptr, TEXT("UActionBase::Initialize InActionBaseData is invalid"));

		ActionName = &InActionName;
		Owner = InOwner;
		ActionBaseData = InActionBaseData;
		ActionBaseShowData = InActionBaseShowData;
		State = EActionState::Wait;
		StepPassedTime = 0.0f;
		RemainCoolDown = 0.0f;
		bIsComplete = false;

		ObjectPoolType = ObjectPoolTypeIndex<TActionObject>::GetType();

		return static_cast<TActionObject*>(this);
	}
	virtual void Casting(TArray<TObjectPtr<AActor>> Targets);
	virtual void Exec(TArray<TObjectPtr<AActor>> Targets);
	virtual void ExecInterval();
	virtual void Finish(TArray<TObjectPtr<AActor>> Targets);
	virtual void Cooldown();
	virtual void Complete();	
	virtual void Cancel();

	EActionState GetState() const { return State; }
	const FName& GetActionName() const { return *ActionName; }
	const struct FActionBaseData* GetActionBaseData() const { return ActionBaseData; }	
	const EObjectPoolType GetObjectPoolType() const { return ObjectPoolType; }
	const EDataTable GetDataType() const { return DataType; }
	const EDataTable GetShowDataType() const { return ShowDataType; }

	virtual void OnPooled() override;
	virtual void OnReturnedToPool() override;

	bool IsCompleted() const 
	{ 
		checkf(!(bIsComplete && State != EActionState::Wait), TEXT("UActionBase::bIsComplete State [ %d ]"), static_cast<int32>(State));
		return bIsComplete; 
	}

	// 외부로직으로 액션을 사용할지 여부를 결정하는 필터
	typedef void (*StepNotiFuncPtr)(const UActionBase*, EActionState);

#if WITH_EDITOR
	void EditorLoadAllShow(
		TObjectPtr<UShowSequencer>& OutCastShow,
		TObjectPtr<UShowSequencer>& OutExecShow,
		TObjectPtr<UShowSequencer>& OutFinishShow);
#endif

protected:
	TObjectPtr<UShowSequencer> LoadShow(const FSoftObjectPath& ShowPath);
	void PlayShow(TObjectPtr<UShowSequencer> ShowSequencer);
	
	EActionState State = EActionState::Wait;

protected:
	TObjectPtr<AActor> Owner;
	const struct FActionBaseData* ActionBaseData;
	const struct FActionBaseShowData* ActionBaseShowData;

	TObjectPtr<UShowSequencer> CastShow = nullptr;
	TObjectPtr<UShowSequencer> ExecShow = nullptr;
	TObjectPtr<UShowSequencer> FinishShow = nullptr;

	float StepPassedTime = 0.0f;
	float RemainCoolDown = 0.0f;

	const FName* ActionName;
	bool bIsComplete = false;

	EObjectPoolType ObjectPoolType = EObjectPoolType::Max;
	EDataTable DataType = EDataTable::Max;
	EDataTable ShowDataType = EDataTable::Max;

public:
	StepNotiFuncPtr StepNotiFunc = nullptr;

};
