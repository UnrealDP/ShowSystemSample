// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionBase.h"
#include "Data/ActionBaseData.h"
#include "Data/ActionBaseShowData.h"
#include "RunTime/ShowSequencer.h"


void UActionBase::OnPooled()
{
	ActionName = nullptr;
	Owner = nullptr;
	ActionBaseData = nullptr;
	ActionBaseShowData = nullptr;
	State = EActionState::Wait;
	StepPassedTime = 0.0f;
	RemainCoolDown = 0.0f;
	bIsComplete = false;
}

void UActionBase::OnReturnedToPool()
{
	if (ShowPlayer)
	{

		// 에디터에서는 UActionBase::EditorLoadAllShow 로 PlayShow 하지 않고 Load를 먼저 해두기 때문에
#if WITH_EDITOR		
		if (CastShowPtr)
		{
			if (ShowPlayer->HasShowSequencer(Owner, CastShowPtr))
			{
				ShowPlayer->DisposeSoloShow(Owner, CastShowPtr);
			}
			CastShowPtr = nullptr;
		}

		if (ExecShowPtr)
		{
			if (ShowPlayer->HasShowSequencer(Owner, ExecShowPtr))
			{
				ShowPlayer->DisposeSoloShow(Owner, ExecShowPtr);
			}
			ExecShowPtr = nullptr;
		}

		if (FinishShowPtr)
		{
			if (ShowPlayer->HasShowSequencer(Owner, FinishShowPtr))
			{
				ShowPlayer->DisposeSoloShow(Owner, FinishShowPtr);
			}
			FinishShowPtr = nullptr;
		}
#else
		if (CastShowPtr)
		{
			ShowPlayer->DisposeSoloShow(Owner, CastShowPtr);
			CastShowPtr = nullptr;
		}

		if (ExecShowPtr)
		{
			ShowPlayer->DisposeSoloShow(Owner, ExecShowPtr);
			ExecShowPtr = nullptr;
		}

		if (FinishShowPtr)
		{
			ShowPlayer->DisposeSoloShow(Owner, FinishShowPtr);
			FinishShowPtr = nullptr;
		}
#endif
	}

	ActionName = nullptr;
	Owner = nullptr;
	ActionBaseData = nullptr;
	ActionBaseShowData = nullptr;
	State = EActionState::Wait;
	StepPassedTime = 0.0f;
	RemainCoolDown = 0.0f;
	bIsComplete = false;
}

void UActionBase::Tick(float DeltaTime)
{
	// Wait는 아무런 로직이 없는 대기 상태
	if (State != EActionState::Wait)
	{
		StepPassedTime += DeltaTime;

		if (RemainCoolDown > 0.0f)
		{
			RemainCoolDown -= DeltaTime;

			// 쿨타임은 어느 스탭에서 시작할지 옵션이다.
			// 쿨타임이 끝났다고 스킬이 끝난 것은 아님
			if (RemainCoolDown <= 0.0f)
			{
				RemainCoolDown = 0.0f;
				Complete();
			}
		}
	}
}

UShowSequencer* UActionBase::NewShowSequencer(EActionState EActionStatem)
{
	checkf(ActionBaseShowData, TEXT("UActionBase::NewShowSequencer ActionBaseShowData is invalid"));

	if (ShowPlayer)
	{
		switch (EActionStatem)
		{
		case EActionState::Cast:
			if (CastShowPtr)
			{
				ShowPlayer->DisposeSoloShow(Owner, CastShowPtr);
				CastShowPtr = nullptr;
			}

			if (ActionBaseShowData->CastShow.IsValid())
			{
				return CastShowPtr = ShowPlayer->NewShowSequencer(Owner, ActionBaseShowData->CastShow);
			}
			break;
		case EActionState::Exec:
			if (ExecShowPtr)
			{
				ShowPlayer->DisposeSoloShow(Owner, ExecShowPtr);
				ExecShowPtr = nullptr;
			}

			if (ActionBaseShowData->ExecShow.IsValid())
			{
				return ExecShowPtr = ShowPlayer->NewShowSequencer(Owner, ActionBaseShowData->ExecShow);
			}
			break;
		case EActionState::Finish:
			if (FinishShowPtr)
			{
				ShowPlayer->DisposeSoloShow(Owner, FinishShowPtr);
				FinishShowPtr = nullptr;
			}

			if (ActionBaseShowData->FinishShow.IsValid())
			{
				return FinishShowPtr = ShowPlayer->NewShowSequencer(Owner, ActionBaseShowData->FinishShow);
			}
			break;
		default:
			break;
		}
	}
	return nullptr;
}

void UActionBase::PlayShow(UShowSequencer* ShowSequencerPtr)
{
	checkf(ShowSequencerPtr, TEXT("UActionBase::PlayShow ShowSequencerPtr is invalid"));

	if (ShowPlayer)
	{
		ShowPlayer->PlaySoloShow(Owner, ShowSequencerPtr);
	}
}

void UActionBase::Casting(TArray<TObjectPtr<AActor>> Targets)
{
	checkf(Owner != nullptr, TEXT("UActionBase::Cast Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Cast ActionBaseData is invalid"));

	if (ActionBaseShowData && ActionBaseShowData->CastShow.IsValid())
	{
		if (!CastShowPtr)
		{
			NewShowSequencer(EActionState::Cast);
		}
		checkf(CastShowPtr, TEXT("UActionBase::Cast CastShowPtr Load Fail"));

		PlayShow(CastShowPtr);
	}

	if (ActionBaseData->CooldownStart == EActionState::Cast)
	{
		RemainCoolDown = ActionBaseData->Cooldown;
	}

	StepPassedTime = 0.0f;
	State = EActionState::Cast;
	bIsComplete = false;
}

// Exec 가 존재하지 않는 액션은 없음 (Cast는 0일 경우 스킵될 수도 있음)
void UActionBase::Exec(TArray<TObjectPtr<AActor>> Targets)
{
	checkf(Owner != nullptr, TEXT("UActionBase::Exec Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Exec ActionBaseData is invalid"));

	// 서버에서 Cast가 0이면 Cast 패킷 전송이 필요 없다.
	// 바로 Exec 상태로 넘어가기 때문에 Cast에 Show가 있으면 Play 한다.
	// 물론 Exec에도 Show가 있으면 Play 한다. 중복 연출 나오면 안되면 이건 연출쪽에서 관리해야함!!!
	if (State == EActionState::Wait)
	{
		if (ActionBaseShowData && ActionBaseShowData->CastShow.IsValid())
		{
			if (!CastShowPtr)
			{
				NewShowSequencer(EActionState::Cast);
			}
			checkf(CastShowPtr, TEXT("UActionBase::Exec CastShowPtr Load Fail"));

			PlayShow(CastShowPtr);
		}
	}

	if (ActionBaseShowData && ActionBaseShowData->ExecShow.IsValid())
	{
		if (!ExecShowPtr)
		{
			NewShowSequencer(EActionState::Exec);
		}
		checkf(ExecShowPtr, TEXT("UActionBase::Exec ExecShowPtr Load Fail"));

		PlayShow(ExecShowPtr);
	}

	StepPassedTime = 0.0f;
	State = EActionState::Exec;
	bIsComplete = false;

	if (ActionBaseData->CooldownStart == EActionState::Exec)
	{
		RemainCoolDown = ActionBaseData->Cooldown;
	}
}

void UActionBase::ExecInterval()
{
	checkf(Owner != nullptr, TEXT("UActionBase::ExecInterval Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::ExecInterval ActionBaseData is invalid"));
}

// 서버에서 Finish를 명시적으로 패킷 전송해줘야 하는가??
// 이건 서버 구조에 따라 달라질 수 있으니 서버와 협의 필요
// 지금은 서버에서 Finish를 보내준다고 가정하고 만들겠음
// 가상 서버 로직은 ActionExecutor 에서 구현하겠음
void UActionBase::Finish(TArray<TObjectPtr<AActor>> Targets)
{
	checkf(Owner != nullptr, TEXT("UActionBase::Finish Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Finish ActionBaseData is invalid"));

	if (ActionBaseShowData && ActionBaseShowData->FinishShow.IsValid())
	{
		if (!FinishShowPtr)
		{
			NewShowSequencer(EActionState::Finish);
		}
		checkf(FinishShowPtr, TEXT("UActionBase::Finish FinishShowPtr Load Fail"));

		PlayShow(FinishShowPtr);
	}

	StepPassedTime = 0.0f;
	State = EActionState::Finish;

	if (ActionBaseData->CooldownStart == EActionState::Finish)
	{
		RemainCoolDown = ActionBaseData->Cooldown;
	}

	// 쿨타임이 이미 끝났던가 혹은 쿨타임이 없는 경우
	// Cooldown 상태로 넘어가지 않고 바로 Complete 해서 Wait 상태로 넘어간다.
	if (RemainCoolDown <= 0.0f)
	{
		Complete();
	}
}

void UActionBase::Cooldown()
{
	checkf(Owner != nullptr, TEXT("UActionBase::Cooldown Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Cooldown ActionBaseData is invalid"));

	StepPassedTime = 0.0f;

	if (RemainCoolDown > 0.0f)
	{
		State = EActionState::Cooldown;
	}
	else
	{
		Complete();
	}
}

void UActionBase::Complete()
{
	checkf(Owner != nullptr, TEXT("UActionBase::Complete Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Complete ActionBaseData is invalid"));

	StepPassedTime = 0.0f;
	RemainCoolDown = 0.0f;
	State = EActionState::Wait;
	bIsComplete = true;
}

void UActionBase::Cancel()
{
	checkf(Owner != nullptr, TEXT("UActionBase::Finish Owner is invalid"));
	checkf(ActionBaseData != nullptr, TEXT("UActionBase::Finish ActionBaseData is invalid"));

	if (ShowPlayer)
	{
		if (CastShowPtr)
		{
			ShowPlayer->StopSoloShow(Owner, CastShowPtr);
		}

		if (ExecShowPtr)
		{
			ShowPlayer->StopSoloShow(Owner, ExecShowPtr);
		}

		if (FinishShowPtr)
		{
			ShowPlayer->StopSoloShow(Owner, FinishShowPtr);
		}
	}

	StepPassedTime = 0.0f;
	RemainCoolDown = 0.0f;
	State = EActionState::Wait;
	bIsComplete = true;
}