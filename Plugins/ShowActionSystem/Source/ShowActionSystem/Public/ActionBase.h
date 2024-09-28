// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionBase.generated.h"

UENUM(BlueprintType)
enum class EActionState : uint8
{
	Wait	UMETA(DisplayName = "Action State Wait"),
	Cast	UMETA(DisplayName = "Action State Cast"),
	Exec	UMETA(DisplayName = "Action State Exec"),
	Finish	UMETA(DisplayName = "Action State Finish"),
	Max		UMETA(DisplayName = "Max Types")
};

/**
 * 
 */
UCLASS()
class SHOWACTIONSYSTEM_API UActionBase : public UObject
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EActionState State = EActionState::Wait;
};
