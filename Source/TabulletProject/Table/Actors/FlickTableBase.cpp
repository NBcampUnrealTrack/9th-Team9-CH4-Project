// Fill out your copyright notice in the Description page of Project Settings.


#include "FlickTableBase.h"
#include "Components/SceneComponent.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"
#include "TabulletProject/Table/Components/TableFallJudgeComponent.h"
#include "TabulletProject/Table/Components/TablePieceSpawnComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerState.h"

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
	
	PieceSpawner = CreateDefaultSubobject<UTablePieceSpawnComponent>(TEXT("PieceSpawner"));
	
	Player1SpawnOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Player1SpawnOrigin"));
	Player1SpawnOrigin->SetupAttachment(RootComponent);

	Player2SpawnOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Player2SpawnOrigin"));
	Player2SpawnOrigin->SetupAttachment(RootComponent);

	Player3SpawnOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Player3SpawnOrigin"));
	Player3SpawnOrigin->SetupAttachment(RootComponent);

	Player4SpawnOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Player4SpawnOrigin"));
	Player4SpawnOrigin->SetupAttachment(RootComponent);
	
	SpecialPieceSpawnOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("SpecialPieceSpawnOrigin"));
	SpecialPieceSpawnOrigin->SetupAttachment(RootComponent);
}

int32 AFlickTableBase::SpawnNormalPiecesForPlayers(const TArray<APlayerState*>& Players, int32 PiecesPerPlayer)
{
	if (!HasAuthority() || !IsValid(PieceSpawner) || PiecesPerPlayer <= 0)
	{
		return 0;
	}

	USceneComponent* SpawnOrigins[] =
	{
		Player1SpawnOrigin,
		Player2SpawnOrigin,
		Player3SpawnOrigin,
		Player4SpawnOrigin
	};

	const int32 PlayerCount = FMath::Min(Players.Num(), 4);
	int32 TotalSpawnedCount = 0;

	for (int32 PlayerIndex = 0; PlayerIndex < PlayerCount; ++PlayerIndex)
	{
		APlayerState* PlayerState = Players[PlayerIndex];
		USceneComponent* SpawnOrigin = SpawnOrigins[PlayerIndex];

		if (!IsValid(PlayerState) || !IsValid(SpawnOrigin))
		{
			continue;
		}

		TotalSpawnedCount += PieceSpawner->SpawnNormalPieces(PlayerState, SpawnOrigin->GetComponentTransform(), PiecesPerPlayer);
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d normal table pieces for %d players"), TotalSpawnedCount, PlayerCount);

	return TotalSpawnedCount;
}

int32 AFlickTableBase::SpawnSpecialPieces(int32 PieceCount)
{
	if (!HasAuthority() || !IsValid(PieceSpawner) || !IsValid(SpecialPieceSpawnOrigin) || PieceCount <= 0)
	{
		return 0;
	}

	const int32 SpawnedCount = PieceSpawner->SpawnSpecialPieces(SpecialPieceSpawnOrigin->GetComponentTransform(), PieceCount);

	UE_LOG(LogTemp, Log, TEXT("Spawned %d special table pieces"), SpawnedCount);

	return SpawnedCount;
}

int32 AFlickTableBase::ResetTablePieces()
{
	if (!HasAuthority())
	{
		return 0;
	}

	int32 RemovedPieceCount = 0;

	// 라운드에 등록된 총알만 제거.
	for (ATableBulletPiece* Piece : RegisteredPieces)
	{
		if (IsValid(Piece))
		{
			Piece->Destroy();
			++RemovedPieceCount;
		}
	}

	RegisteredPieces.Empty();
	ActiveFlickPlayerState = nullptr;
	SettledElapsedTime = 0.0f;
	bMonitoringPieceMovement = false;
	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("Reset table pieces: removed %d pieces"), RemovedPieceCount);

	return RemovedPieceCount;
}

void AFlickTableBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bMonitoringPieceMovement)
	{
		return;
	}

	RegisteredPieces.RemoveAll([](const TObjectPtr<ATableBulletPiece>& Piece)
{
	return !IsValid(Piece);
});

	bool bAnyPieceMoving = false;

	for (ATableBulletPiece* RegisteredPiece : RegisteredPieces)
	{
		if (RegisteredPiece->IsMoving(LinearSpeedThreshold, AngularSpeedThreshold))
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
	ActiveFlickPlayerState = nullptr;

	UE_LOG(LogTemp, Log, TEXT("All table pieces have settled"));

	OnTablePiecesSettled.Broadcast();
}

void AFlickTableBase::HandlePieceEnteredFallJudge(ATableBulletPiece* FallenPiece)
{
	if (!HasAuthority() || !IsValid(FallenPiece) || !RegisteredPieces.Contains(FallenPiece))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Table piece fell: %s"), *FallenPiece->GetName());

	// 게임 모드용 낙하 이벤트
	OnTablePieceFell.Broadcast(FallenPiece, FallenPiece->GetOwningPlayerState());

	if (FallenPiece->GetPieceType() == ETablePieceType::Special && IsValid(ActiveFlickPlayerState))
	{
		const EWeaponType RewardWeaponType = FallenPiece->GetRewardWeaponType();

		OnSpecialPieceCaptured.Broadcast(ActiveFlickPlayerState, RewardWeaponType);

		UE_LOG(LogTemp, Log, TEXT("Special table piece captured by %s, weapon type: %d"), *ActiveFlickPlayerState->GetPlayerName(), static_cast<int32>(RewardWeaponType));
	}
	
	FallenPiece->MarkAsOut();			// 판정 나면 TableBulletPiece의 MarkAsOut으로 아웃 처리
	UnregisterPiece(FallenPiece);		// 등록된 총알에서 제거
}

bool AFlickTableBase::TryApplyFlick(ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (!IsValid(Piece) || !RegisteredPieces.Contains(Piece))
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
		ActiveFlickPlayerState = Piece->GetOwningPlayerState();
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
