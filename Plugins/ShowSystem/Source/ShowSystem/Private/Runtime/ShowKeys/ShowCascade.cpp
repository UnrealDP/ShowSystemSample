// Fill out your copyright notice in the Description page of Project Settings.


#include "RunTime/ShowKeys/ShowCascade.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/Lifetime/ParticleModuleLifetime.h"

FString UShowCascade::GetTitle()
{
    if (!ParticleSystem)
    {
        return "ShowAnimStatic";
    }
    return ParticleSystem->GetName();
}

void UShowCascade::Initialize()
{
    checkf(ShowKey, TEXT("UShowCascade::Initialize ShowKey is invalid"));

    CascadeKeyPtr = static_cast<const FShowCascadeKey*>(ShowKey);
    checkf(CascadeKeyPtr, TEXT("UShowCascade::Initialize CascadeKeyPtr is invalid [ %d ]"), static_cast<int>(ShowKey->KeyType));

    if (!ParticleComponent)
    {
        if (CascadeKeyPtr->CascadeParticleAsset.IsNull())
        {
            ShowKeyState = EShowKeyState::ShowKey_End;
            return;
        }

        if (!CascadeKeyPtr->CascadeParticleAsset.IsValid())
        {
            CascadeKeyPtr->CascadeParticleAsset.LoadSynchronous();
        }

        ParticleSystem = CascadeKeyPtr->CascadeParticleAsset.Get();
        if (!ParticleSystem)
        {
            checkf(false, TEXT("UShowCascade::Initialize ParticleSystem is invalid"));
            ShowKeyState = EShowKeyState::ShowKey_End;
            return;
        }
    }
}

float UShowCascade::GetLength()
{
    if (!ParticleSystem)
    {
        return 0.0f;
    }

    float Length = 0.0f;
    for (UParticleEmitter* Emitter : ParticleSystem->Emitters)
    {
        if (!Emitter)
        {
            continue;
        }

        for (UParticleLODLevel* LODLevel : Emitter->LODLevels)
        {
            if (LODLevel && LODLevel->RequiredModule)
            {
                UParticleModuleRequired* RequiredModule = LODLevel->RequiredModule;

                // 루프 수 및 딜레이 설정 확인 (RequiredModule에서 가져옴)
                int32 EmitterLoops = RequiredModule->EmitterLoops ? RequiredModule->EmitterLoops : 1;
                float StartDelay = RequiredModule->EmitterDelay;
                float Duration = RequiredModule->EmitterDuration;

                // 에미터 자체의 전체 시간 계산
                float EmitterTotalTime = StartDelay + (Duration * EmitterLoops);

                // Burst 리스트에서 최대 Time 값 계산
                float MaxBurstTime = 0.0f;
                for (const FParticleBurst& Burst : RequiredModule->BurstList)
                {
                    MaxBurstTime = FMath::Max(MaxBurstTime, Burst.Time);
                }

                // 각 파티클의 최대 Lifetime 계산
                float MaxLifetime = 0.0f;
                for (UParticleModule* Module : LODLevel->Modules)
                {
                    if (UParticleModuleLifetime* LifetimeModule = Cast<UParticleModuleLifetime>(Module))
                    {
                        float MinOut = 0.0f;
                        LifetimeModule->Lifetime.GetOutRange(MinOut, MaxLifetime);
                        break;
                    }
                }

                // Burst와 Lifetime을 고려한 시간 계산
                float BurstAndLifetimeTime = StartDelay + MaxBurstTime + MaxLifetime;

                // 두 값 중 더 큰 값을 선택
                float TotalTime = FMath::Max(EmitterTotalTime, BurstAndLifetimeTime);

                // 가장 긴 시간 업데이트
                Length = FMath::Max(Length, TotalTime);
            }
        }
    }

    return Length / ShowKey->PlayRate;
}

void UShowCascade::Dispose()
{
    if (ParticleComponent)
    {
        ParticleComponent->DeactivateSystem();
        ParticleComponent->UnregisterComponent();
        ParticleComponent->DestroyComponent();
        ParticleComponent = nullptr;
    }

    CascadeKeyPtr = nullptr;
}

void UShowCascade::Reset()
{
    Dispose();
    Initialize();
}

void UShowCascade::Play()
{
    AActor* ShowOwner = GetShowOwner();
    checkf(ShowOwner, TEXT("UShowCascade::Play ShowOwner is invalid"));
    checkf(ParticleSystem, TEXT("UShowCascade::Play ParticleSystem is invalid"));

    ParticleComponent = NewObject<UParticleSystemComponent>(ShowOwner);
    if (ParticleComponent)
    {
        ShowOwner->AddInstanceComponent(ParticleComponent);
        ParticleComponent->RegisterComponent();
        ParticleComponent->SetTemplate(ParticleSystem);
        ParticleComponent->bAutoDestroy = false;
        ParticleComponent->bAutoActivate = false;
    }
    else
    {
        checkf(false, TEXT("UShowAnimStatic::Initialize ParticleComponent is invalid"));
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    USkeletalMeshComponent* SkeletalMeshComp = ShowOwner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("UShowCascade::PlayAttachToBone SkeletalMeshComponent is invalid"));

    // Case 1: 모든 트랜스폼 규칙이 KeepRelativeThenFollowRoot이 아닐 때
    if (!(CascadeKeyPtr->LocationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot) &&
        !(CascadeKeyPtr->RotationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot) &&
        !(CascadeKeyPtr->ScaleRule == EAttachmentRuleEx::KeepWorldThenFollowRoot))
    {
        DefaultEngineAttach(ShowOwner);
    }
    // Case 2: 모든 트랜스폼 규칙이 KeepRelativeThenFollowRoot일 때
    else if (
        (CascadeKeyPtr->LocationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot) &&
        (CascadeKeyPtr->RotationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot) &&
        (CascadeKeyPtr->ScaleRule == EAttachmentRuleEx::KeepWorldThenFollowRoot))
	{
		AllFollowRoot(ShowOwner);
	}
    // Case 3: 일부 트랜스폼 규칙이 KeepRelativeThenFollowRoot인 경우
	else
	{
		PartialFollowRoot(ShowOwner);
	}

    ParticleComponent->ActivateSystem(true);
    ParticleComponent->CustomTimeDilation = ShowKey->PlayRate * CachedTimeScale;

    // OnSystemFinished 델리게이트에 종료 이벤트 연결
    if (!ParticleComponent->OnSystemFinished.IsAlreadyBound(this, &UShowCascade::OnParticleSystemFinished))
    {
        ParticleComponent->OnSystemFinished.AddDynamic(this, &UShowCascade::OnParticleSystemFinished);
    }

    ShowKeyState = EShowKeyState::ShowKey_Playing;
}

// Case 1: 모든 트랜스폼 규칙이 KeepRelativeThenFollowRoot이 아닐 때
void UShowCascade::DefaultEngineAttach(AActor* ShowOwner)
{
    USkeletalMeshComponent* SkeletalMeshComp = ShowOwner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("SkeletalMeshComponent is invalid"));

    checkf(SkeletalMeshComp->DoesSocketExist(CascadeKeyPtr->AttachBone),
        TEXT("UShowCascade::DefaultEngineAttach AttachBone '%s' does not exist on SkeletalMeshComponent"),
        *CascadeKeyPtr->AttachBone.ToString());

    FAttachmentTransformRules AttachmentRules(
        static_cast<EAttachmentRule>(CascadeKeyPtr->LocationRule),
        static_cast<EAttachmentRule>(CascadeKeyPtr->RotationRule),
        static_cast<EAttachmentRule>(CascadeKeyPtr->ScaleRule),
        CascadeKeyPtr->bWeldSimulatedBodies
    );

    ParticleComponent->SetRelativeLocation(CascadeKeyPtr->LocalLocation);
    ParticleComponent->SetRelativeRotation(CascadeKeyPtr->LocalRotation);
    ParticleComponent->SetRelativeScale3D(CascadeKeyPtr->LocalScale);

    ParticleComponent->SetUseAutoManageAttachment(true);
    ParticleComponent->SetAutoAttachmentParameters(
        SkeletalMeshComp,
        CascadeKeyPtr->AttachBone,
        static_cast<EAttachmentRule>(CascadeKeyPtr->LocationRule),
        static_cast<EAttachmentRule>(CascadeKeyPtr->RotationRule),
        static_cast<EAttachmentRule>(CascadeKeyPtr->ScaleRule)
    );
}

// Case 2: 모든 트랜스폼 규칙이 KeepRelativeThenFollowRoot일 때
void UShowCascade::AllFollowRoot(AActor* ShowOwner)
{
    USkeletalMeshComponent* SkeletalMeshComp = ShowOwner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("SkeletalMeshComponent is invalid"));

    // AttachBone의 트랜스폼을 Root 기준으로 가져옴
    FTransform AttachBoneTransform = SkeletalMeshComp->GetSocketTransform(CascadeKeyPtr->AttachBone, RTS_World);
    FTransform RootTransform = ShowOwner->GetActorTransform();
    FTransform RelativeToRoot = AttachBoneTransform.GetRelativeTransform(RootTransform);

    // AttachBone 상대 트랜스폼과 Local 값을 조합하여 파티클의 트랜스폼 설정
    ParticleComponent->SetRelativeLocation(RelativeToRoot.GetLocation() + CascadeKeyPtr->LocalLocation);
    ParticleComponent->SetRelativeRotation(RelativeToRoot.GetRotation().Rotator() + CascadeKeyPtr->LocalRotation);
    ParticleComponent->SetRelativeScale3D(RelativeToRoot.GetScale3D() * CascadeKeyPtr->LocalScale);

    ParticleComponent->SetUseAutoManageAttachment(true);
    ParticleComponent->SetAutoAttachmentParameters(
        ShowOwner->GetRootComponent(),
        "root",
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative,
        EAttachmentRule::KeepRelative
    );
}

// Case 3: 일부 트랜스폼 규칙이 KeepRelativeThenFollowRoot인 경우
void UShowCascade::PartialFollowRoot(AActor* ShowOwner)
{
    USkeletalMeshComponent* SkeletalMeshComp = ShowOwner->FindComponentByClass<USkeletalMeshComponent>();
    checkf(SkeletalMeshComp, TEXT("SkeletalMeshComponent is invalid"));

    // ParticleAttachmentState 구조체 할당
    ParticleAttachmentState = MakeUnique<FParticleAttachmentState>();

    // AttachBone의 트랜스폼을 Root 기준으로 가져옴
    FTransform AttachBoneTransform = SkeletalMeshComp->GetSocketTransform(CascadeKeyPtr->AttachBone, RTS_World);
    FTransform RootTransform = ShowOwner->GetActorTransform();

    EAttachmentRule LocationAttachmentRule = static_cast<EAttachmentRule>(CascadeKeyPtr->LocationRule);
    EAttachmentRule RotationAttachmentRule = static_cast<EAttachmentRule>(CascadeKeyPtr->RotationRule);
    EAttachmentRule ScaleAttachmentRule = static_cast<EAttachmentRule>(CascadeKeyPtr->ScaleRule);

    if (CascadeKeyPtr->LocationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot)
    {
        ParticleAttachmentState->bIsLocationFollowRoot = true;
        ParticleAttachmentState->InitialParentLocation = AttachBoneTransform.GetLocation() + CascadeKeyPtr->LocalLocation;
        ParticleAttachmentState->InitialRootLocation = RootTransform.GetLocation();
        LocationAttachmentRule = EAttachmentRule::KeepWorld;
        ParticleComponent->SetWorldLocation(ParticleAttachmentState->InitialParentLocation);
    }
    else
    {
        ParticleComponent->SetRelativeLocation(CascadeKeyPtr->LocalLocation);
    }

    if (CascadeKeyPtr->RotationRule == EAttachmentRuleEx::KeepWorldThenFollowRoot)
    {
        ParticleAttachmentState->bIsRotationFollowRoot = true;
        ParticleAttachmentState->InitialParentRotation = AttachBoneTransform.GetRotation().Rotator() + CascadeKeyPtr->LocalRotation;
        ParticleAttachmentState->InitialRootRotation = RootTransform.GetRotation().Rotator();
        RotationAttachmentRule = EAttachmentRule::KeepWorld;
        ParticleComponent->SetWorldRotation(ParticleAttachmentState->InitialParentRotation);
    }
    else
    {
        ParticleComponent->SetRelativeRotation(CascadeKeyPtr->LocalRotation);
    }

    if (CascadeKeyPtr->ScaleRule == EAttachmentRuleEx::KeepWorldThenFollowRoot)
    {
        ParticleAttachmentState->bIsScaleFollowRoot = true;
        ParticleAttachmentState->InitialParentScale = AttachBoneTransform.GetScale3D() * CascadeKeyPtr->LocalScale;
        ParticleAttachmentState->InitialRootScale = RootTransform.GetScale3D();
        ScaleAttachmentRule = EAttachmentRule::KeepWorld;
        ParticleComponent->SetWorldScale3D(ParticleAttachmentState->InitialParentScale);
    }
    else
    {
        ParticleComponent->SetRelativeScale3D(CascadeKeyPtr->LocalScale);
    }

    FAttachmentTransformRules AttachmentRules(
        LocationAttachmentRule,
        RotationAttachmentRule,
        ScaleAttachmentRule,
        CascadeKeyPtr->bWeldSimulatedBodies
    );

    ParticleComponent->SetUseAutoManageAttachment(true);
    ParticleComponent->SetAutoAttachmentParameters(
        SkeletalMeshComp,
        CascadeKeyPtr->AttachBone,
        LocationAttachmentRule,
        RotationAttachmentRule,
        ScaleAttachmentRule
    );
}

void UShowCascade::OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent)
{
    if (!ParticleComponent)
    {
        ShowKeyState = EShowKeyState::ShowKey_End;
        return;
    }

    if (ParticleComponent == FinishedComponent)
    {
        if (ShowKeyState != EShowKeyState::ShowKey_Playing &&
            ShowKeyState != EShowKeyState::ShowKey_Pause)
        {
            checkf(false, TEXT("UShowCascade::OnParticleSystemFinished ShowKeyState is not Playing"));
        }

        if (ParticleComponent->OnSystemFinished.IsAlreadyBound(this, &UShowCascade::OnParticleSystemFinished))
        {
            ParticleComponent->OnSystemFinished.RemoveDynamic(this, &UShowCascade::OnParticleSystemFinished);
        }

        ShowKeyState = EShowKeyState::ShowKey_End;
    }
}

void UShowCascade::Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime)
{
    if (!ParticleComponent || ShowKeyState != EShowKeyState::ShowKey_Playing || !ParticleAttachmentState)
    {
        return;
    }

    AActor* ShowOwner = GetShowOwner();
    if (!ShowOwner)
    {
        return;
    }

    // Root의 현재 상태를 가져옴
    FVector CurrentRootLocation = ShowOwner->GetActorLocation();
    FRotator CurrentRootRotation = ShowOwner->GetActorRotation();
    FVector CurrentRootScale = ShowOwner->GetActorScale3D();

    // Root의 변화 계산
    FVector RootLocationOffset = CurrentRootLocation - ParticleAttachmentState->InitialRootLocation;
    FRotator RootRotationOffset = CurrentRootRotation - ParticleAttachmentState->InitialRootRotation;
    FVector RootScaleOffset = CurrentRootScale / ParticleAttachmentState->InitialRootScale;

    // 트랜스폼 업데이트 (각 요소별로 판단)
    if (ParticleAttachmentState->bIsLocationFollowRoot)
    {
        FVector NewLocation = ParticleAttachmentState->InitialParentLocation + RootLocationOffset;
        ParticleComponent->SetWorldLocation(NewLocation);
    }

    if (ParticleAttachmentState->bIsRotationFollowRoot)
    {
        FRotator NewRotation = ParticleAttachmentState->InitialParentRotation + RootRotationOffset;
        ParticleComponent->SetWorldRotation(NewRotation);
    }

    if (ParticleAttachmentState->bIsScaleFollowRoot)
    {
        FVector NewScale = ParticleAttachmentState->InitialParentScale * RootScaleOffset;
        ParticleComponent->SetWorldScale3D(NewScale);
    }
}

void UShowCascade::ApplyTimeScale(float FinalTimeScale)
{
	if (ParticleComponent)
	{
		ParticleComponent->CustomTimeDilation = ShowKey->PlayRate * FinalTimeScale;
	}
}