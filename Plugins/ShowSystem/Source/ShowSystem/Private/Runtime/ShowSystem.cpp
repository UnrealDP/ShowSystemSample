// Fill out your copyright notice in the Description page of Project Settings.

#include "RunTime/ShowSystem.h"
#include "InstancedStruct.h"
#include "RunTime/ShowKeys/ShowAnimStatic.h"
#include "RunTime/ShowKeys/ShowCascade.h"

#define LOCTEXT_NAMESPACE "ShowSystem"

EObjectPoolType ShowSystem::GetShowKeyPoolType(EShowKeyType InShowKeyType)
{
    switch (InShowKeyType)
    {
    case EShowKeyType::ShowKey_Anim:
        return EObjectPoolType::ObjectPool_ShowKeyAnim;
    case EShowKeyType::ShowKey_Cascade:
        return EObjectPoolType::ObjectPool_ShowKeyCascade;
    default:
        checkf(false, TEXT("ShowSystem::GetShowKeyPoolType Invalid EShowKeyType: [ %d ]"), static_cast<int32>(InShowKeyType));
        return EObjectPoolType::Max;
    }
}

UScriptStruct* ShowSystem::GetShowKeyStaticStruct(EShowKeyType InShowKeyType)
{
    switch (InShowKeyType)
    {
    case EShowKeyType::ShowKey_Anim:
        return FShowAnimStaticKey::StaticStruct();
    case EShowKeyType::ShowKey_Cascade:
        return FShowCascadeKey::StaticStruct();
    default:
        checkf(false, TEXT("ShowSystem::GetShowKeyStaticStruct Invalid EShowKeyType: [ %d ]"), static_cast<int32>(InShowKeyType));
        return nullptr;
    }
}

void ShowSystem::NotifyShowKeyChange(UShowBase* ShowBasePtr, FName PropertyName)
{
    if (!ShowBasePtr)
    {
        return;
    }

    if (ShowBasePtr->IsA<UShowAnimStatic>())
    {
        if (PropertyName.IsEqual("AnimSequenceAsset"))
        {
            ShowBasePtr->ExecuteReset();
        }
    }
    else if (ShowBasePtr->IsA<UShowCascade>())
    {
        if (PropertyName.IsEqual("CascadeParticleAsset"))
        {
            ShowBasePtr->ExecuteReset();
        }
    }
}

bool ShowSystem::ValidateRuntimeShowKey(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt)
{
    if (ShowBasePtr->IsA(UShowAnimStatic::StaticClass()))
    {
        return ValidateShowAnimStatic(Owner, ShowBasePtr, ErrTxt);
    }
    else if (ShowBasePtr->IsA(UShowCascade::StaticClass()))
	{
		return ValidateShowCascade(Owner, ShowBasePtr, ErrTxt);
	}

    FString ClassName = ShowBasePtr->GetClass() ? ShowBasePtr->GetClass()->GetName() : TEXT("Unknown");
    ErrTxt = FText::Format(
        LOCTEXT("ShowBaseNone", "[ValidateRuntimeShowKey] The StaticClass definition is missing for the class: !! {0} !!"),
        FText::FromString(ClassName)
    );
    return false;
}

bool ShowSystem::ValidateShowAnimStatic(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt)
{
    UShowAnimStatic* ShowAnimStatic = Cast<UShowAnimStatic>(ShowBasePtr);

    TWeakObjectPtr<UAnimSequenceBase> AnimSequenceBase = ShowAnimStatic->GetAnimSequenceBase();
    if (!AnimSequenceBase.IsValid())
    {
        ErrTxt = LOCTEXT("ShowBaseNone", "[UShowAnimStatic] An animation is missing for one or more keys.");
        return false;
    }

    USkeletalMeshComponent* SkeletalMeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMeshComp)
    {
        ErrTxt = LOCTEXT("ShowBaseNone", "[UShowAnimStatic] None SkeletalMeshComp");
        return false;
    }

    if (!SkeletalMeshComp->AnimClass)
    {
        ErrTxt = LOCTEXT("ShowBaseNone", "[UShowAnimStatic] None SkeletalMeshComp->AnimClass");
        return false;
    }

    UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance();
    if (!AnimInstance)
    {
        ErrTxt = LOCTEXT("ShowBaseNone", "[UShowAnimStatic] None AnimInstance");
        return false;
    }

    return true;
}

bool ShowSystem::ValidateShowCascade(AActor* Owner, UShowBase* ShowBasePtr, FText& ErrTxt)
{
    UShowCascade* ShowCascade = Cast<UShowCascade>(ShowBasePtr);

    TWeakObjectPtr<UParticleSystem> ParticleSystem = ShowCascade->GetParticleSystem();
    if (!ParticleSystem.IsValid())
    {
        ErrTxt = LOCTEXT("ShowBaseNone", "[UShowCascade] UParticleSystem is null");
        return false;
    }

    return true;
}

#undef LOCTEXT_NAMESPACE