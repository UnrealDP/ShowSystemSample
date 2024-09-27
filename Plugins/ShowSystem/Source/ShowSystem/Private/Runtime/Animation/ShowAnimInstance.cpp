// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/Animation/ShowAnimInstance.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AnimNodes/AnimNode_LayeredBoneBlend.h"

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
        if (OwnerComponent->SkeletalMesh)
        {
            Skeleton = OwnerComponent->SkeletalMesh->GetSkeleton();
        }
    }

    OwnerActor = OwnerComponent->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    MovementComponent = OwnerActor->FindComponentByClass<UCharacterMovementComponent>();
}

void UShowAnimInstance::PlayAnimation(UAnimSequenceBase* NewAsset, bool bLooping, float BlendOutTriggerTime, float InTimeToStartMontageAt)
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
            BlendOutTriggerTime,
            InTimeToStartMontageAt);

        if (!DynamicMontage)
        {
            UE_LOG(LogTemp, Error, TEXT("UShowAnimInstance::PlayAnimation DynamicMontage is null [ %s ]"), *NewAsset->GetFName().ToString());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("UShowAnimInstance::PlayAnimation Play DynamicMontage [ %s ]"), *NewAsset->GetFName().ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UShowAnimInstance::PlayAnimation AnimData is not found"));
    }
}