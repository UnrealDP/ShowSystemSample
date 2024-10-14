// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowAnimStatic.h"
#include "RunTime/Animation/ShowAnimInstance.h"

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
float UShowAnimStatic::InitializeAssetLength()
{
    if (!AnimSequenceBase)
	{
		return 0.0f;
	}

    if (ShowKey->Length > 0.0f)
	{
		return ShowKey->Length;
	}

	return AnimSequenceBase->GetPlayLength();
}

void UShowAnimStatic::Dispose()
{
    AnimStaticKeyPtr = nullptr;
    SkeletalMeshComp = nullptr;
    AnimSequenceBase = nullptr;
}

void UShowAnimStatic::Play() 
{
    AActor* Owner = GetOwner();
    checkf(Owner, TEXT("UShowAnimStatic::Play Owner is invalid"));
    checkf(AnimSequenceBase, TEXT("UShowAnimStatic::Play AnimSequence is invalid"));

    // Actor의 SkeletalMeshComponent 가져오기
    SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("UShowAnimStatic::Play SkeletalMeshComponent is invalid"));

    if (!SkeletalMeshComp->AnimClass)
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    if (UShowAnimInstance* ShowAnimInstance = Cast<UShowAnimInstance>(SkeletalMeshComp->GetAnimInstance()))
    {
        ShowAnimInstance->PlayAnimation(AnimSequenceBase, AnimStaticKeyPtr->IsLoop);
    }
    else
    {
        SkeletalMeshComp->PlayAnimation(AnimSequenceBase, AnimStaticKeyPtr->IsLoop);
    }
}

void UShowAnimStatic::Reset() 
{
    AnimSequenceBase = nullptr;
    Initialize();
}