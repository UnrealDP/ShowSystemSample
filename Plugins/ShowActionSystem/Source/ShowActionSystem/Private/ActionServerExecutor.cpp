// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionServerExecutor.h"

void UActionServerExecutor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (State == EActionState::Wait)
	{
		return;
	}

	switch (State)
	{
	case EActionState::Cast:
		if (StepPassedTime >= ActionBaseData->CastDuration)
		{
			Exec();
		}
		break;
	case EActionState::Exec:
		if (StepPassedTime >= ActionBaseData->ExecDuration)
		{
			Finish();
		}
		break;
	case EActionState::Finish:
		break;
	default:
		break;
	}
}

void UActionServerExecutor::DoAction()
{
	Casting();
}