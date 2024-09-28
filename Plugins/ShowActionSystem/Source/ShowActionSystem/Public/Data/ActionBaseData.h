#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionBaseData.generated.h"

USTRUCT(BlueprintType)
struct FActionBaseData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float CastDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float ExecDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float FinishDuration = 0.0f;
};
