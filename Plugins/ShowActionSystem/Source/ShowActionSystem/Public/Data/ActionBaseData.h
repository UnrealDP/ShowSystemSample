#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ActionBaseData.generated.h"

UENUM(BlueprintType)
enum class EActionState : uint8
{
	Wait		UMETA(DisplayName = "Action State Wait"),
	Cast		UMETA(DisplayName = "Action State Cast"),
	Exec		UMETA(DisplayName = "Action State Exec"),
	Finish		UMETA(DisplayName = "Action State Finish"),
	Cooldown	UMETA(DisplayName = "Action State Cooldown"),
	Max			UMETA(DisplayName = "Max Types")
};


USTRUCT(BlueprintType)
struct FActionBaseData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	int32 Priority = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float CastDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float ExecDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	int ExecInterval = 0; // 밀리세컨트 단위 이며 100ms 이상이어야 함

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	EActionState CooldownStart = EActionState::Exec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action Base Data")
	float Cooldown = 0.0f;	
};
