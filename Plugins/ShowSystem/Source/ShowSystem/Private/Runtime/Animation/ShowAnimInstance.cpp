// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/Animation/ShowAnimInstance.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"
#include "Runtime/ShowKeys/ShowAnimStatic.h"

void UShowAnimInstance::BeginDestroy()
{
	OwnerComponent = nullptr;
	Skeleton = nullptr;
	OwnerActor = nullptr;
	MovementComponent = nullptr;

	Super::BeginDestroy();
}

void UShowAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerComponent = GetSkelMeshComponent();
    if(!OwnerComponent)
	{
		return;
	}
    else
    {
        USkeletalMesh* SkeletalMesh = OwnerComponent->GetSkeletalMeshAsset();
        if (SkeletalMesh)
        {
            Skeleton = SkeletalMesh->GetSkeleton();
        }
    }

    OwnerActor = OwnerComponent->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    MovementComponent = OwnerActor->FindComponentByClass<UCharacterMovementComponent>();
}

void UShowAnimInstance::PlayAnimation(const UShowAnimStatic& ShowAnimStatic, float PlayRate)
{
    UAnimSequenceBase* AnimSequenceBase = ShowAnimStatic.GetAnimSequenceBase();
    checkf(AnimSequenceBase, TEXT("UShowAnimInstance::PlayAnimation AnimSequenceBase is invalid"));

    const FShowAnimStaticKey* ShowAnimStaticKey = ShowAnimStatic.GetAnimStaticKey();
    checkf(ShowAnimStaticKey, TEXT("UShowAnimInstance::PlayAnimation ShowAnimStaticKey is invalid"));

    UAnimMontage* AnimMontage = PlayAnimation(
        AnimSequenceBase, 
        ShowAnimStaticKey->LoopCount, 
        ShowAnimStaticKey->BlendOutTriggerTime, 
        ShowAnimStaticKey->InTimeToStartMontageAt, 
        PlayRate);

    if (AnimMontage)
    {
    }
}

UAnimMontage* UShowAnimInstance::PlayAnimation(UAnimSequenceBase* NewAsset, int32 LoopCount, float BlendOutTriggerTime, float InTimeToStartMontageAt, float PlayRate)
{
    checkf(NewAsset, TEXT("UShowAnimInstance::PlayAnimation NewAsset is invalid"));
    checkf(AnimContainer, TEXT("UShowAnimInstance::PlayAnimation AnimContainer is invalid"));

    FSoftObjectPath NewAssetPath = FSoftObjectPath(NewAsset);

    if (const FAnimData* FoundData = AnimContainer->FindAnimData(NewAssetPath))
    {
        checkf(Skeleton->ContainsSlotName(FoundData->Slot), TEXT("UShowAnimInstance::PlayAnimation -> Slot [ %s ] not exist"), *FoundData->Slot.ToString());

        const FMontageBlendSettings* BlendInSettings = nullptr;
        const FMontageBlendSettings* BlendOutSettings = nullptr;
        if (const FAnimBlendData* FoundBlendData = AnimContainer->FindAnimBlendData(NewAssetPath))
        {
            BlendInSettings = &FoundBlendData->BlendInSettings;
            BlendOutSettings = &FoundBlendData->BlendOutSettings;
        }
        else
        {
            BlendInSettings = &AnimContainer->DefaultAnimBlendData.BlendInSettings;
            BlendOutSettings = &AnimContainer->DefaultAnimBlendData.BlendOutSettings;
        }

        UAnimMontage* DynamicMontage = PlaySlotAnimationAsDynamicMontage_WithBlendSettings(
            NewAsset,
            FoundData->Slot,
            *BlendInSettings,
            *BlendOutSettings,
            LoopCount,
            BlendOutTriggerTime,
            InTimeToStartMontageAt);
        
        if (DynamicMontage)
        {
            Montage_SetPlayRate(DynamicMontage, PlayRate);
            UE_LOG(LogTemp, Log, TEXT("UShowAnimInstance::PlayAnimation Play DynamicMontage [ %s ]"), *NewAsset->GetFName().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UShowAnimInstance::PlayAnimation DynamicMontage is null [ %s ]"), *NewAsset->GetFName().ToString());
        }

        return DynamicMontage;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UShowAnimInstance::PlayAnimation AnimData is not found"));
        return nullptr;
    }
}
