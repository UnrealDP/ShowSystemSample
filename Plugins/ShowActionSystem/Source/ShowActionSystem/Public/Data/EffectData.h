#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EDataTable.h"
#include "ActionEffectBaseData.h"
#include "EffectData.generated.h"

DATATABLE_TYPE_INDEX(FEffectData, EDataTable::EffectData)

USTRUCT(BlueprintType)
struct FEffectData : public FActionEffectBaseData
{
	GENERATED_BODY()

public:
};
