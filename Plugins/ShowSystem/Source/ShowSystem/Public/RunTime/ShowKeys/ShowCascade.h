// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RunTime/ShowBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "ShowCascade.generated.h"

UENUM()
enum class EAttachmentRuleEx
{
    /** Keeps current relative transform as the relative transform to the new parent. */
    KeepRelative = static_cast<int32>(EAttachmentRule::KeepRelative),

    /** Automatically calculates the relative transform such that the attached component maintains the same world transform. */
    KeepWorld = static_cast<int32>(EAttachmentRule::KeepWorld),

    /** Snaps transform to the attach point */
    SnapToTarget = static_cast<int32>(EAttachmentRule::SnapToTarget),

     /**
     * [주요이슈] 어태치 된후의 Root의 값을 적용하는 것이기 때문에 최초 어태치 될때 Root의 정보를 저장하고 Tick에서 변화된 값을 파티클에 적용해야 할지 고민 필요
     * 
     * 어태치 될때 부모의 값을 적용 후 부모의 값을 더이상 적용 안하고 대신 붙은 후에는 root의 값을 적용함 
     * 1. Location : 파티클이 생성될 때 손 위치에서 생성했지만 그 후로는 생성했던 위치에 있으면서 손의 위치 변화는 적용 안하고 대신 캐릭터(Root)의 이동은 적용함
     *   ex) 최초 캐릭터(Root)의 위치가 50,10,0 이고 손의 위치가 80,10,0 일 때 (Root로 부터 손의 상대 위치는 30,0,0) 파티클이 어태치되면서 생성되면 80,10,0 에 생성
     *     캐릭터(Root)가 x축으로 100만큼 이동하면서 손을 y축으로 50 이동하면
     *     캐릭터(Root)의 위치는 150,10,0 이 되고 
     *     손 월드상의 위치는 180,60,0 이 되지만 
     *     파티클은 어태치 된 후의 손 위치는 적용을 안하기 떄문에 어태치 된후의 손의 y축 50이동은 적용 안하고
     *     캐릭터(Root)의 위치 값만 적용하기 때문에 파티클의 최종 월드 위치는 180,10,0 에 있음
     *
     * 2. Rotation : 파티클이 생성될 때 손 회전을 적용해서 생성했지만 그 후로는 손의 회전 변화는 적용 안하고 대신 캐릭터(Root)의 회전은 적용함
     *   ex) 최초 캐릭터(Root)의 회전값이 50,10,0 이고 손의 회전값이 80,10,0 일 때 (Root로 부터 손의 상대 회전값은 30,0,0) 파티클이 어태치되면서 생성되면 회전값 80,10,0 으로 생성
     *     캐릭터(Root)가 x축으로 100만큼 회전하면서 손을 y축으로 50 회전하면
     *     캐릭터(Root)의 회전은 150,10,0 이 되고 
     *     손 월드상의 회전은 180,60,0 이 되지만 
     *     파티클은 어태치 된 후의 손 회전은 적용을 안하기 떄문에 어태치 된후의 손의 y축 50회전은 적용 안하고
     *     캐릭터(Root)의 회전만 적용하기 때문에 파티클의 최종 월드 회전은 180,10,0 임
     *
     * 3. Scale : 파티클이 생성될 때 손 스케일을 적용해서 생성했지만 그 후로는 손의 스케일 변화는 적용 안하고 대신 캐릭터(Root)의 스케일은 적용함
     *   ex) 최초 캐릭터(Root)의 스케일값이 2, 1.5, 1 이고 손의 스케일값이 4, 1.5, 1 일 때 (Root로 부터 손의 상대 스케일값은 2, 1, 1) 파티클이 어태치되면서 생성되면 스케일값 4, 1.5, 1 으로 생성
     *     캐릭터(Root)가 x 1.5만큼 스케일하면서 손을 y 2 스케일하면
     *     캐릭터(Root)의 스케일은 3, 1.5, 1 이 되고 
     *     손 최종 스케일은 6, 3, 1 이 되지만 
     *     파티클은 어태치 된 후의 손 스케일은 적용을 안하기 떄문에 어태치 된후의 손의 y 2 스케일은 적용 안하고
     *     캐릭터(Root)의 회전만 적용하기 때문에 파티클의 최종 최종 스케일은 6, 1.5, 1 임
     */
    KeepWorldThenFollowRoot,
};

// 트랜스폼 상태를 저장하는 구조체
struct FParticleAttachmentState
{
    // 초기 부모(손)의 위치, 회전, 스케일
    FVector InitialParentLocation;
    FRotator InitialParentRotation;
    FVector InitialParentScale;

    // Root의 초기 위치, 회전, 스케일
    FVector InitialRootLocation;
    FRotator InitialRootRotation;
    FVector InitialRootScale;

    // FollowRoot 옵션 상태
    bool bIsLocationFollowRoot = false;
    bool bIsRotationFollowRoot = false;
    bool bIsScaleFollowRoot = false;

    FParticleAttachmentState()
        : InitialParentLocation(FVector::ZeroVector)
        , InitialParentRotation(FRotator::ZeroRotator)
        , InitialParentScale(FVector::OneVector)
        , InitialRootLocation(FVector::ZeroVector)
        , InitialRootRotation(FRotator::ZeroRotator)
        , InitialRootScale(FVector::OneVector)
    {}
};

USTRUCT(BlueprintType)
struct FShowCascadeKey : public FShowKey
{
    GENERATED_BODY()

    FShowCascadeKey()
    {
        KeyType = EShowKeyType::ShowKey_Cascade;
    }

    UPROPERTY(EditAnywhere, Category = "CascadeKey")
    TSoftObjectPtr<UParticleSystem> CascadeParticleAsset;

    UPROPERTY(EditAnywhere, Category = "CascadeKey Offset")
    FVector LocalLocation = FVector::Zero();

    UPROPERTY(EditAnywhere, Category = "CascadeKey Offset")
    FRotator LocalRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, Category = "CascadeKey Offset")
    FVector LocalScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, Category = "CascadeKey Attach")
    FName AttachBone = "root";

    UPROPERTY(EditAnywhere, Category = "CascadeKey Attach")
    /** The rule to apply to location when attaching */
    EAttachmentRuleEx LocationRule = EAttachmentRuleEx::KeepRelative;

    UPROPERTY(EditAnywhere, Category = "CascadeKey Attach")
    /** The rule to apply to rotation when attaching */
    EAttachmentRuleEx RotationRule = EAttachmentRuleEx::KeepRelative;

    UPROPERTY(EditAnywhere, Category = "CascadeKey Attach")
    /** The rule to apply to scale when attaching */
    EAttachmentRuleEx ScaleRule = EAttachmentRuleEx::KeepRelative;

    UPROPERTY(EditAnywhere, Category = "CascadeKey Attach")
    /** Whether to weld simulated bodies together when attaching */
    bool bWeldSimulatedBodies = false;
};


/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UShowCascade : public UShowBase
{
	GENERATED_BODY()
	
public:
    TWeakObjectPtr<UParticleSystem> GetParticleSystem() const
    {
        return TWeakObjectPtr<UParticleSystem>(ParticleSystem.Get());
    }

    virtual FString GetTitle() override;
    virtual float GetLength() override;

protected:
    virtual void Initialize() override;
    virtual void Dispose() override;
    virtual void Play() override;
    virtual void Reset();
    virtual void Pause() override {};
    virtual void UnPause() override {};
    virtual void Tick(float ScaleDeltaTime, float SystemDeltaTime, float BasePassedTime) override;
    virtual void ApplyTimeScale(float FinalTimeScale) override;

    UFUNCTION()
    void OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent);

private:
    void DefaultEngineAttach(AActor* ShowOwner);
    void AllFollowRoot(AActor* ShowOwner);
    void PartialFollowRoot(AActor* ShowOwner);

private:
    const FShowCascadeKey* CascadeKeyPtr = nullptr;

    UPROPERTY()
    TObjectPtr<UParticleSystem> ParticleSystem = nullptr;

    UPROPERTY()
    TObjectPtr<UParticleSystemComponent> ParticleComponent = nullptr;

    TUniquePtr<FParticleAttachmentState> ParticleAttachmentState = nullptr;
};
