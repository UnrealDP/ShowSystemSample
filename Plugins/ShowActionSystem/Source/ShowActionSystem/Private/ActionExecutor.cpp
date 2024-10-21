// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionExecutor.h"
#include "Data/ActionBaseData.h"

void UActionExecutor::BeginDestroy()
{
	ActionBasePtr = nullptr;
	Super::BeginDestroy();
}

void UActionExecutor::Tick(float DeltaTime)
{
	if (!ActionBasePtr)
	{
		return;
	}

	EActionState ActionState = ActionBasePtr->GetState();
	if (ActionState == EActionState::Wait)
	{
		return;
	}

	const FActionBaseData* ActionBaseData = ActionBasePtr->GetActionBaseData();
	if (!ActionBaseData)
	{
		return;
	}

	PassedTime += DeltaTime;
	StepPassedTime += DeltaTime;

	switch (ActionState)
	{
	case EActionState::Cast:
		if(StepPassedTime >= ActionBaseData->CastDuration)
		{
			//ActionBase->Exec();
			StepPassedTime = 0.0f;
		}
		break;
	case EActionState::Exec:
		if (StepPassedTime >= ActionBaseData->ExecDuration)
		{
			//ActionBase->Finish();
			StepPassedTime = 0.0f;
		}
		break;
	case EActionState::Finish:
		/*if (StepPassedTime >= ActionBaseData->FinishDuration)
		{
			ActionBase->Cooldown();
			StepPassedTime = 0.0f;
		}*/
		break;
	default:
		break;
	}
}

