// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ObjectPool/Pooled.h"
#include "Data/ActionBaseData.h"
#include "ActionBase.generated.h"

class UShowSequencer;

/**
 * 액션의 기본 기능을 정의한 클래스
 * 하나의 액션을 정의함
 * 액션을 진행하는 서버 시뮬레이션 로직은 ActionExecutor 에 구현
 * 데디케이트 서버를 사용한다면 ActionExecutor 로 가능할지 구조를 다시 고려해야함
 */
UCLASS()
class SHOWACTIONSYSTEM_API UActionBase : public UObject, public IPooled
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);

	virtual UActionBase* Initialize(TObjectPtr<AActor> InOwner, const FName& InActionName, const struct FActionBaseData* InActionBaseData, const struct FActionBaseShowData* InActionBaseShowData);
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

	virtual void OnPooled() override;
	virtual void OnReturnedToPool() override;

	bool IsCompleted() const 
	{ 
		checkf(!(bIsComplete && State != EActionState::Wait), TEXT("UActionBase::bIsComplete State [ %d ]"), static_cast<int32>(State));
		return bIsComplete; 
	}

	// 외부로직으로 액션을 사용할지 여부를 결정하는 필터
	typedef void (*StepNotiFuncPtr)(const UActionBase*, EActionState);

protected:
	TObjectPtr<UShowSequencer> PlayShow(const FSoftObjectPath& ShowPath);
	
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

public:
	StepNotiFuncPtr StepNotiFunc = nullptr;

};
