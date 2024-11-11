// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EShowKeyType : uint8
{
    ShowKey_Anim UMETA(DisplayName = "Animation Key"),
    ShowKey_Cascade UMETA(DisplayName = "Cascade Key"),
    ShowKey_Niagara UMETA(DisplayName = "Niagara Key"),
    ShowKey_CamSequence UMETA(DisplayName = "CamSequence Key"),
    ShowKey_CamShake UMETA(DisplayName = "CamShake Key"),
    Max   UMETA(DisplayName = "Max Types")
};
