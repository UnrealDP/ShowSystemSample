#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionBaseShowData.generated.h"

USTRUCT(BlueprintType)
struct FActionBaseShowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Show Data")
	FSoftObjectPath CastShow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Show Data")
	FSoftObjectPath ExecShow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Show Data")
	FSoftObjectPath FinishShow;
};
