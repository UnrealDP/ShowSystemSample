// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowAnimStatic.h"
#include "RunTime/Animation/ShowAnimInstance.h"

void UShowAnimStatic::Initialize(const FShowKey& InShowKey) 
{
    AnimStaticKeyPtr = static_cast<const FShowAnimStaticKey*>(&InShowKey);
    checkf(AnimStaticKeyPtr, TEXT("UShowAnimStatic::Initialize AnimStaticKey is invalid [ %d ]"), static_cast<int>(InShowKey.KeyType));

    // AnimSequenceClass로부터 UAnimSequenceBase 인스턴스 생성
    AnimSequenceBase = AnimStaticKeyPtr->AnimSequenceClass->GetDefaultObject<UAnimSequenceBase>();
    checkf(AnimSequenceBase, TEXT("UShowAnimStatic::Initialize AnimSequence is invalid"));
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