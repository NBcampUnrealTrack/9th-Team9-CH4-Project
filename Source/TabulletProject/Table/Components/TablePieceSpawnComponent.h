// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TablePieceSpawnComponent.generated.h"


class ATableBulletPiece;
class APlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TABULLETPROJECT_API UTablePieceSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTablePieceSpawnComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn")		// 생성할 BP 클래스
	TSubclassOf<ATableBulletPiece> NormalPieceClass;
	
	int32 SpawnSpecialPieces(const FTransform& SpawnOrigin, int32 PieceCount);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn")
	TArray<TSubclassOf<ATableBulletPiece>> SpecialPieceClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1.0"))
	float SpecialPieceSpacing = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1.0"))		// 총알 사이 간격
	float PieceSpacing = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1"))
	int32 MaxPiecesPerRow = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1.0"))
	float RowSpacing = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1"))
	int32 MaxSpecialPiecesPerRow = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "1.0"))
	float SpecialRowSpacing = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Spawn", meta = (ClampMin = "0.0"))		// 바둑판 위로 얼마나 위인지
	float SpawnHeightOffset = 5.0f;
	
	int32 SpawnNormalPieces(APlayerState* OwningPlayer, const FTransform& SpawnOrigin, int32 PieceCount);	
};
