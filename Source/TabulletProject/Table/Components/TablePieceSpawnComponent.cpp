// Fill out your copyright notice in the Description page of Project Settings.


#include "TablePieceSpawnComponent.h"

#include "GameFramework/PlayerState.h"
#include "TabulletProject/Table/Actors/FlickTableBase.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"


// Sets default values for this component's properties
UTablePieceSpawnComponent::UTablePieceSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
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
	const FVector OriginLocation = SpawnOrigin.GetLocation();
	const FVector SideDirection = SpawnOrigin.GetUnitAxis(EAxis::Y);
	const FVector UpDirection = SpawnOrigin.GetUnitAxis(EAxis::Z);
	const float CenterOffset = static_cast<float>(PieceCount - 1) * 0.5f;

	for (int32 Index = 0; Index < PieceCount; ++Index)
	{
		const float SideOffset = (static_cast<float>(Index) - CenterOffset) * PieceSpacing;
		const FVector SpawnLocation = OriginLocation + SideDirection * SideOffset + UpDirection * SpawnHeightOffset;
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