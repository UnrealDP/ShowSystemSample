#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EDataTable.h"
#include "ActionBaseData.h"
#include "SkillData.generated.h"

DATATABLE_TYPE_INDEX(FSkillData, EDataTable::SkillData)

USTRUCT(BlueprintType)
struct FSkillData : public FActionBaseData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float FinishDuration = 0.0f;

};
