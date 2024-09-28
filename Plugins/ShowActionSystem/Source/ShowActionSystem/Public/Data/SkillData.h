#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionBaseData.h"
#include "SkillData.generated.h"

USTRUCT(BlueprintType)
struct FSkillData : public FActionBaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString Name = "";

};
