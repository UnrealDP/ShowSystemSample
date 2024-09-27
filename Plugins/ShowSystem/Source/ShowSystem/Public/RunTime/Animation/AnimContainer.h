// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AnimContainer.generated.h"

UENUM(BlueprintType)
enum class EAnimAlias : uint8
{
    Sturn UMETA(DisplayName = "Stun State"),  // 스턴 상태 애니메이션
    Max   UMETA(DisplayName = "Max Types"),
};

USTRUCT(BlueprintType)
struct FAnimData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData", meta = (DisplayAfter = "AnimSequences Obj Path"))
    FSoftObjectPath AnimSequences;

    // 상하체 분리 등 어떤 타입으로 할 것인지 Anim Slot Manager에 쎄팅한 Slot 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimData", meta = (DisplayAfter = "Anim Slot"))
    FName Slot = "DefaultSlot";
};

USTRUCT(BlueprintType)
struct FAnimBlendData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimBlendData", meta = (DisplayAfter = "Blend In"))
    FMontageBlendSettings BlendInSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimBlendData", meta = (DisplayAfter = "Blend Out"))
    FMontageBlendSettings BlendOutSettings;
};

/**
 * 
 */
UCLASS()
class SHOWSYSTEM_API UAnimContainer : public UObject
{
	GENERATED_BODY()
	
public:
    const FAnimData* FindAnimData(const FSoftObjectPath& NewAssetPath) const
	{
		return AnimPureDatas.FindByPredicate([&NewAssetPath](const FAnimData& AnimData)
			{
				return AnimData.AnimSequences == NewAssetPath;
			});
	}

	const FAnimBlendData* FindAnimBlendData(const FSoftObjectPath& NewAssetPath) const
	{
		return AnimBlendDatas.Find(NewAssetPath);
	}

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    FAnimBlendData DefaultAnimBlendData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TArray<FAnimData> AnimPureDatas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TMap<EAnimAlias, FAnimData> AnimAliasDatas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Data")
    TMap<FSoftObjectPath, FAnimBlendData> AnimBlendDatas;
};
