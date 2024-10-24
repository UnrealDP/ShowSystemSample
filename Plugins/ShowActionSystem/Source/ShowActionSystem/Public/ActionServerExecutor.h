// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionSkill.h"
#include "ActionServerExecutor.generated.h"

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEM_API UActionServerExecutor : public UActionSkill
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	void DoAction();
};
