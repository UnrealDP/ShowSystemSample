// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionComponent.generated.h"

class UActionBase;
class UActionComponent;
struct FActionBaseData;
struct FActionBaseShowData;

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

	void InitializeActionPool(const TArray<FName>& ActionNames);
	void ClearActionPool();

	UActionBase* DoActionPool(const FName& ActionName, ActionFilterFuncPtr ActionFilter = nullptr);
	UActionBase* DoAction(const FName& ActionName, ActionFilterFuncPtr ActionFilter = nullptr);

	const UActionBase* GetMainAction() const { return MainAction; }
	
private:
	UActionBase* MainAction = nullptr;

	TMap<FName, UActionBase*> ActionPool;

	TMap<FName, UActionBase*> OneShotActions;
};
