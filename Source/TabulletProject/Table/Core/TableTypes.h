// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TableTypes.generated.h"

UENUM(BlueprintType)
enum class ETablePieceType : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Special	UMETA(DisplayName = "Special")
};

UENUM(BlueprintType)
enum class ETablePieceState : uint8
{
	OnTable		UMETA(DisplayName = "On Table"),
	Falling		UMETA(DisplayName = "Falling"),
	Out			UMETA(DisplayName = "Out")
};