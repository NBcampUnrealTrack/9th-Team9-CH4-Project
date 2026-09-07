// Fill out your copyright notice in the Description page of Project Settings.


#include "TablePieceSpawnComponent.h"

#include "GameFramework/PlayerState.h"
#include "TabulletProject/Table/Actors/FlickTableBase.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"

namespace
{
	FVector CalculateMultiRowSpawnLocation(const FTransform& SpawnOrigin, int32 PieceIndex, int32 PieceCount, int32 MaxPiecesPerRow, float SideSpacing, float ForwardSpacing, float HeightOffset)
	{
		const int32 SafeMaxPiecesPerRow = FMath::Max(MaxPiecesPerRow, 1);
		const int32 RowIndex = PieceIndex / SafeMaxPiecesPerRow;
		const int32 IndexInRow = PieceIndex % SafeMaxPiecesPerRow;
		const int32 RemainingPieceCount = PieceCount - RowIndex * SafeMaxPiecesPerRow;
		const int32 PiecesInRow = FMath::Min(RemainingPieceCount, SafeMaxPiecesPerRow);
		const float CenterOffset = static_cast<float>(PiecesInRow - 1) * 0.5f;

		const FVector ForwardDirection = SpawnOrigin.GetUnitAxis(EAxis::X);
		const FVector SideDirection = SpawnOrigin.GetUnitAxis(EAxis::Y);
		const FVector UpDirection = SpawnOrigin.GetUnitAxis(EAxis::Z);
		const float SideOffset = (static_cast<float>(IndexInRow) - CenterOffset) * SideSpacing;
		const float ForwardOffset = static_cast<float>(RowIndex) * ForwardSpacing;

		return SpawnOrigin.GetLocation() + ForwardDirection * ForwardOffset + SideDirection * SideOffset + UpDirection * HeightOffset;
	}
}

UTablePieceSpawnComponent::UTablePieceSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UTablePieceSpawnComponent::SpawnNormalPieces(APlayerState* OwningPlayer, const FTransform& SpawnOrigin, int32 PieceCount)
{
	AFlickTableBase* Table = Cast<AFlickTableBase>(GetOwner());
	UWorld* World = GetWorld();

	if (!IsValid(Table) || !Table->HasAuthority() || !IsValid(World) || !IsValid(OwningPlayer) || !NormalPieceClass || PieceCount <= 0)
	{
		return 0;
	}

	int32 SpawnedCount = 0;

	for (int32 Index = 0; Index < PieceCount; ++Index)
	{
		// 각 줄을 가운데 정렬하고 다음 줄은 바둑판 중앙 방향으로 배치한다.
		const FVector SpawnLocation = CalculateMultiRowSpawnLocation(SpawnOrigin, Index, PieceCount, MaxPiecesPerRow, PieceSpacing, RowSpacing, SpawnHeightOffset);
		const FTransform PieceTransform(SpawnOrigin.GetRotation(), SpawnLocation);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Table;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ATableBulletPiece* SpawnedPiece = World->SpawnActor<ATableBulletPiece>(NormalPieceClass, PieceTransform, SpawnParameters);

		if (!IsValid(SpawnedPiece))
		{
			continue;
		}

		SpawnedPiece->SetOwningPlayerState(OwningPlayer);

		if (!Table->RegisterPiece(SpawnedPiece))
		{
			SpawnedPiece->Destroy();
			continue;
		}

		++SpawnedCount;
	}

	return SpawnedCount;
}

int32 UTablePieceSpawnComponent::SpawnSpecialPieces(const FTransform& SpawnOrigin, int32 PieceCount)
{
	AFlickTableBase* Table = Cast<AFlickTableBase>(GetOwner());
	UWorld* World = GetWorld();

	if (!IsValid(Table) || !Table->HasAuthority() || !IsValid(World) || SpecialPieceClasses.IsEmpty() || PieceCount <= 0)
	{
		return 0;
	}

	TArray<TSubclassOf<ATableBulletPiece>> ValidSpecialPieceClasses;

	for (const TSubclassOf<ATableBulletPiece>& SpecialPieceClass : SpecialPieceClasses)
	{
		if (SpecialPieceClass)
		{
			ValidSpecialPieceClasses.Add(SpecialPieceClass);
		}
	}

	if (ValidSpecialPieceClasses.IsEmpty())
	{
		return 0;
	}

	int32 SpawnedCount = 0;
	const FVector OriginLocation = SpawnOrigin.GetLocation();
	const FVector SideDirection = SpawnOrigin.GetUnitAxis(EAxis::Y);
	const FVector UpDirection = SpawnOrigin.GetUnitAxis(EAxis::Z);
	const float CenterOffset = static_cast<float>(PieceCount - 1) * 0.5f;

	for (int32 Index = 0; Index < PieceCount; ++Index)
	{
		const int32 RandomClassIndex = FMath::RandRange(0, ValidSpecialPieceClasses.Num() - 1);
		const TSubclassOf<ATableBulletPiece> SelectedClass = ValidSpecialPieceClasses[RandomClassIndex];
		const float SideOffset = (static_cast<float>(Index) - CenterOffset) * SpecialPieceSpacing;
		const FVector SpawnLocation = OriginLocation + SideDirection * SideOffset + UpDirection * SpawnHeightOffset;
		const FTransform PieceTransform(SpawnOrigin.GetRotation(), SpawnLocation);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Table;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ATableBulletPiece* SpawnedPiece = World->SpawnActor<ATableBulletPiece>(SelectedClass, PieceTransform, SpawnParameters);

		if (!IsValid(SpawnedPiece))
		{
			continue;
		}

		if (!Table->RegisterPiece(SpawnedPiece))
		{
			SpawnedPiece->Destroy();
			continue;
		}

		++SpawnedCount;
	}

	return SpawnedCount;
}