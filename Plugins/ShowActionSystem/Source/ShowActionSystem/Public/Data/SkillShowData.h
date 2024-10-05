#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EDataTable.h"
#include "ActionBaseShowData.h"
#include "SkillShowData.generated.h"

DATATABLE_TYPE_INDEX(FSkillShowData, EDataTable::SkillShowData)

USTRUCT(BlueprintType)
struct FSkillShowData : public FActionBaseShowData
{
	GENERATED_BODY()

public:
};
