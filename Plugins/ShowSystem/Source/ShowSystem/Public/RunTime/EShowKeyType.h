// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EShowKeyType : uint8
{
    ShowKey_Anim UMETA(DisplayName = "Animation Key"),
    ShowKey_Particle UMETA(DisplayName = "Particle Key"),
    Max   UMETA(DisplayName = "Max Types")
};
