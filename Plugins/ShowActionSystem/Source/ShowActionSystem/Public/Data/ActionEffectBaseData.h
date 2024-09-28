#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionEffectBaseData.generated.h"

USTRUCT(BlueprintType)
struct FActionEffectBaseData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Effect Base Data")
	FSoftObjectPath Show = nullptr;
};
