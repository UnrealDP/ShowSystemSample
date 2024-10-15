// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowAnimStatic.h"
#include "RunTime/Animation/ShowAnimInstance.h"

FString UShowAnimStatic::GetTitle()
{
    if (!AnimSequenceBase)
	{
		return "ShowAnimStatic";
	}
    return AnimSequenceBase->GetName();
}

void UShowAnimStatic::Initialize() 
{
    checkf(ShowKey, TEXT("UShowAnimStatic::Initialize ShowKey is invalid"));

    AnimStaticKeyPtr = static_cast<const FShowAnimStaticKey*>(ShowKey);
    checkf(AnimStaticKeyPtr, TEXT("UShowAnimStatic::Initialize AnimStaticKey is invalid [ %d ]"), static_cast<int>(ShowKey->KeyType));

    if (!AnimSequenceBase)
    {
        if (AnimStaticKeyPtr->AnimSequenceAsset.IsNull())
        {
            ShowKeyState = EShowKeyState::ShowKey_End;
            return;
        }

        if (!AnimStaticKeyPtr->AnimSequenceAsset.IsValid())
        {
            // 필요한 경우 로드
            AnimStaticKeyPtr->AnimSequenceAsset.LoadSynchronous();
        }

        // AnimSequenceClass로부터 UAnimSequenceBase 인스턴스 생성
        AnimSequenceBase = AnimStaticKeyPtr->AnimSequenceAsset.Get();
        checkf(AnimSequenceBase, TEXT("UShowAnimStatic::Initialize AnimSequence is invalid"));
    }
}

// Initialize 된 후에 호출된다
float UShowAnimStatic::GetShowLength()
{
    if (!AnimSequenceBase)
	{
		return ShowKey->Length;
	}

    OriginalLength = AnimSequenceBase->GetPlayLength();

    if (ShowKey->Length > 0.0f)
	{
		return ShowKey->Length;
	}

	return OriginalLength;
}

void UShowAnimStatic::Dispose()
{
    if (AnimInstance && AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UShowAnimStatic::OnMontageEnded))
    {
        AnimInstance->OnMontageEnded.RemoveDynamic(this, &UShowAnimStatic::OnMontageEnded);
    }

    AnimStaticKeyPtr = nullptr;
    AnimSequenceBase = nullptr;
    AnimMontage = nullptr;
    AnimInstance = nullptr;
    OriginalLength = 0.0f;
}

void UShowAnimStatic::Play() 
{
    AActor* Owner = GetOwner();
    checkf(Owner, TEXT("UShowAnimStatic::Play Owner is invalid"));
    checkf(AnimSequenceBase, TEXT("UShowAnimStatic::Play AnimSequence is invalid"));
    checkf(!AnimInstance, TEXT("UShowAnimStatic::Play AnimInstance need reset"));

    USkeletalMeshComponent* SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("UShowAnimStatic::Play SkeletalMeshComponent is invalid"));

    if (!SkeletalMeshComp->AnimClass)
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    float PlayRate = OriginalLength / Length;
    if (UShowAnimInstance* ShowAnimInstance = Cast<UShowAnimInstance>(SkeletalMeshComp->GetAnimInstance()))
    {
        AnimInstance = ShowAnimInstance;
        checkf(!AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UShowAnimStatic::OnMontageEnded), TEXT("UShowAnimStatic::Play MontageEndedHandler need reset"));

        AnimMontage = ShowAnimInstance->PlayAnimation(
            AnimSequenceBase, 
            AnimStaticKeyPtr->LoopCount,
            AnimStaticKeyPtr->BlendOutTriggerTime,
            AnimStaticKeyPtr->InTimeToStartMontageAt,
            PlayRate);

        ShowAnimInstance->OnMontageEnded.AddDynamic(this, &UShowAnimStatic::OnMontageEnded);
    }
    else if (AnimInstance = SkeletalMeshComp->GetAnimInstance())
    {
        checkf(!AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UShowAnimStatic::OnMontageEnded), TEXT("UShowAnimStatic::Play MontageEndedHandler need reset"));

        AnimMontage = AnimInstance->PlaySlotAnimationAsDynamicMontage(
            AnimSequenceBase, 
            TEXT("DefaultSlot"), 
            0.25f,
            0.25f,
            PlayRate,
            AnimStaticKeyPtr->LoopCount,
            AnimStaticKeyPtr->BlendOutTriggerTime,
            AnimStaticKeyPtr->InTimeToStartMontageAt);

        AnimInstance->OnMontageEnded.AddDynamic(this, &UShowAnimStatic::OnMontageEnded);
    }
    else
    { 
        UE_LOG(LogTemp, Error, TEXT("UShowAnimStatic::Play AnimInstance is invalid"));
    }
}

void UShowAnimStatic::Reset() 
{
    if (AnimInstance && AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UShowAnimStatic::OnMontageEnded))
    {
        AnimInstance->OnMontageEnded.RemoveDynamic(this, &UShowAnimStatic::OnMontageEnded);
    }

    AnimStaticKeyPtr = nullptr;
    AnimSequenceBase = nullptr;
    AnimMontage = nullptr;
    AnimInstance = nullptr;
    OriginalLength = 0.0f;

    Initialize();
}

void UShowAnimStatic::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!AnimMontage)
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    if (ShowKeyState != EShowKeyState::ShowKey_Playing &&
        ShowKeyState != EShowKeyState::ShowKey_Pause)
    {
        AnimMontage = nullptr;
        return;
    }

    if (AnimMontage == Montage)
    {
        if (AnimInstance && AnimInstance->OnMontageEnded.IsAlreadyBound(this, &UShowAnimStatic::OnMontageEnded))
        {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &UShowAnimStatic::OnMontageEnded);
        }

        AnimMontage = nullptr;
        ShowKeyState = EShowKeyState::ShowKey_End;
    }
}

void UShowAnimStatic::Tick(float DeltaTime, float BasePassedTime)
{
    // 애니메이션은 OnMontageEnded 이벤트로 종료 처리
}