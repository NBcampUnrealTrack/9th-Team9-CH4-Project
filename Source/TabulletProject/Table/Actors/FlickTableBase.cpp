// Fill out your copyright notice in the Description page of Project Settings.


#include "FlickTableBase.h"
#include "Components/SceneComponent.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"
#include "TabulletProject/Table/Components/TableFallJudgeComponent.h"
#include "EngineUtils.h"

// Sets default values
AFlickTableBase::AFlickTableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	SetReplicateMovement(false);
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	
	SetRootComponent(SceneRoot);
	
	FallJudge = CreateDefaultSubobject<UTableFallJudgeComponent>(TEXT("FallJudge"));

	FallJudge->SetupAttachment(SceneRoot);

	FallJudge->OnPieceEnteredFallJudge.AddDynamic(this,	&AFlickTableBase::HandlePieceEnteredFallJudge);
}

void AFlickTableBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bMonitoringPieceMovement)
	{
		return;
	}

	bool bAnyPieceMoving = false;

	for (TActorIterator<ATableBulletPiece> PieceIterator(GetWorld()); PieceIterator; ++PieceIterator)
	{
		if (PieceIterator->IsMoving(LinearSpeedThreshold, AngularSpeedThreshold))
		{
			bAnyPieceMoving = true;
			break;
		}
	}

	if (bAnyPieceMoving)
	{
		SettledElapsedTime = 0.0f;
		return;
	}

	SettledElapsedTime += DeltaSeconds;

	if (SettledElapsedTime < RequiredSettledTime)
	{
		return;
	}

	bMonitoringPieceMovement = false;
	SettledElapsedTime = 0.0f;
	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("All table pieces have settled"));

	OnTablePiecesSettled.Broadcast();
}

void AFlickTableBase::HandlePieceEnteredFallJudge(ATableBulletPiece* FallenPiece)
{
	if (!IsValid(FallenPiece))
	{
		return;
	}

	UE_LOG(LogTemp,	Log, TEXT("Table piece fell: %s"), *FallenPiece->GetName());
	
	FallenPiece->MarkAsOut(); // 판정 나면 TableBulletPiece의 MarkAsOut으로 아웃 처리
}

bool AFlickTableBase::TryApplyFlick(ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValid(Piece))
	{
		return false;
	}

	const FVector TableUp = GetActorUpVector();

	const FVector FlatDirection = FVector::VectorPlaneProject( WorldDirection, TableUp).GetSafeNormal();	// 방향. z축 제거

	if (FlatDirection.IsNearlyZero())
	{
		return false;
	}

	const float ClampedPower = FMath::Clamp(NormalizedPower, 0.0f, 1.0f);  // 파워 비율. 마우스 땡기는 만큼으로 측정. 0 - 1 사이

	const FVector Impulse =	FlatDirection * MaxFlickImpulse	* ClampedPower; //  최종 힘. 방향 X 최대 힘 X 파워 비율

	const bool bFlickApplied = Piece->ApplyFlickImpulse(Impulse);

	if (bFlickApplied)
	{
		SettledElapsedTime = 0.0f;
		bMonitoringPieceMovement = true;
		SetActorTickEnabled(true);
	}

	return bFlickApplied;
}

bool AFlickTableBase::RegisterPiece(ATableBulletPiece* Piece)
{
	if (!HasAuthority() || !IsValid(Piece) || Piece->IsOut() || RegisteredPieces.Contains(Piece))
	{
		return false;
	}

	RegisteredPieces.Add(Piece);

	UE_LOG(LogTemp, Log, TEXT("Registered table piece: %s"), *Piece->GetName());

	return true;
}

bool AFlickTableBase::UnregisterPiece(ATableBulletPiece* Piece)
{
	if (!HasAuthority() || !IsValid(Piece))
	{
		return false;
	}

	const int32 RemovedCount = RegisteredPieces.Remove(Piece);

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Unregistered table piece: %s"), *Piece->GetName());
		return true;
	}

	return false;
}

int32 AFlickTableBase::GetRegisteredPieceCount() const
{
	return RegisteredPieces.Num();
}

int32 AFlickTableBase::GetRemainingPieceCountForPlayer(const APlayerState* PlayerState) const
{
	if (PlayerState == nullptr)
	{
		return 0;
	}

	int32 RemainingPieceCount = 0;

	for (const ATableBulletPiece* RegisteredPiece : RegisteredPieces)
	{
		if (IsValid(RegisteredPiece) && !RegisteredPiece->IsOut() && RegisteredPiece->IsOwnedByPlayerState(PlayerState))
		{
			++RemainingPieceCount;
		}
	}

	return RemainingPieceCount;
}

int32 AFlickTableBase::GetRegisteredPieceCountByType(ETablePieceType PieceType) const
{
	int32 MatchingPieceCount = 0;

	for (const ATableBulletPiece* RegisteredPiece : RegisteredPieces)
	{
		if (IsValid(RegisteredPiece) && !RegisteredPiece->IsOut() && RegisteredPiece->GetPieceType() == PieceType)
		{
			++MatchingPieceCount;
		}
	}

	return MatchingPieceCount;
}

/*
 *일반탄이랑 특수탄 남은 갯수 볼 수 있음.
 *GetRegisteredPieceCountByType(ETablePieceType::Normal);
 *GetRegisteredPieceCountByType(ETablePieceType::Special);
 */