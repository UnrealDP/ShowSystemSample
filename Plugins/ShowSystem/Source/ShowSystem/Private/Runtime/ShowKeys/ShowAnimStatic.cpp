// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowAnimStatic.h"

void UShowAnimStatic::Initialize(const FShowKey& InShowKey) 
{
    const FShowAnimStaticKey* AnimStaticKeyPtr = static_cast<const FShowAnimStaticKey*>(&InShowKey);
    checkf(AnimStaticKeyPtr, TEXT("UShowAnimStatic::Initialize AnimStaticKey is invalid [ %d ]"), static_cast<int>(InShowKey.KeyType));

    // AnimSequenceClass로부터 UAnimSequence 인스턴스 생성
    AnimationAsset = AnimStaticKeyPtr->AnimSequenceClass->GetDefaultObject<UAnimationAsset>();
    checkf(AnimationAsset, TEXT("UShowAnimStatic::Initialize AnimSequence is invalid"));
}

void UShowAnimStatic::Play() 
{
    AActor* Owner = GetOwner();
    checkf(Owner, TEXT("UShowAnimStatic::Play Owner is invalid"));
    checkf(AnimationAsset, TEXT("UShowAnimStatic::Play AnimSequence is invalid"));

    // Actor의 SkeletalMeshComponent 가져오기
    USkeletalMeshComponent* SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("UShowAnimStatic::Play SkeletalMeshComponent is invalid"));

    // 두 번째 인자는 반복 여부 (false = 반복하지 않음)
    SkeletalMeshComp->PlayAnimation(AnimationAsset, true);
}