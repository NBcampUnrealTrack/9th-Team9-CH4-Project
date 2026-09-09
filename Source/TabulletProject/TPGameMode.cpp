// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameMode.h"

#include "Character/TPCharacter.h"
#include "TPGameState.h"
#include "TPPlayerHUD.h"
#include "TPPlayerController.h"
#include "TPPlayerState.h"
#include "Component/AmmoComponent.h"
#include "Component/HealthComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerStart.h"
#include "TabulletProject/Table/Actors/FlickTableBase.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"

ATPGameMode::ATPGameMode()
{
	bDelayedStart = true;
	GameStateClass = ATPGameState::StaticClass();
	PlayerControllerClass = ATPPlayerController::StaticClass();
	PlayerStateClass = ATPPlayerState::StaticClass();
	DefaultPawnClass = ATPCharacter::StaticClass();
	HUDClass = ATPPlayerHUD::StaticClass();
}

void ATPGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (NumPlayers >= RequiredPlayerCount)
	{
		ErrorMessage = TEXT("The match is full.");
		return;
	}

	if (GetMatchState() != MatchState::WaitingToStart)
	{
		ErrorMessage = TEXT("The match has already started.");
	}
}

void ATPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		for (int32 PlayerIndex = 0; PlayerIndex < TPGameState->PlayerArray.Num(); ++PlayerIndex)
		{
			if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(TPGameState->PlayerArray[PlayerIndex]))
			{
				TPPlayerState->SetPlayerIndex(PlayerIndex);
			}
		}

		TPGameState->SetMatchPhase(NumPlayers >= RequiredPlayerCount ? ETabulletMatchPhase::Starting : ETabulletMatchPhase::WaitingForPlayers);
	}

	BeginStartCountdown();
}

void ATPGameMode::Logout(AController* Exiting)
{
	APlayerState* ExitingPlayerState = Exiting ? Exiting->PlayerState : nullptr;
	const bool bWasCurrentTurnPlayer = ExitingPlayerState && ExitingPlayerState == GetCurrentTurnPlayerState();
	const int32 RemovedTurnIndex = ExitingPlayerState ? TurnOrder.IndexOfByKey(ExitingPlayerState) : INDEX_NONE;

	if (ATPPlayerState* TPPlayerState = Exiting ? Exiting->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetEliminated(true);
	}

	Super::Logout(Exiting);

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			for (int32 PlayerIndex = 0; PlayerIndex < TPGameState->PlayerArray.Num(); ++PlayerIndex)
			{
				if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(TPGameState->PlayerArray[PlayerIndex]))
				{
					TPPlayerState->SetPlayerIndex(PlayerIndex);
				}
			}

			TPGameState->SetMatchPhase(NumPlayers >= RequiredPlayerCount ? ETabulletMatchPhase::Starting : ETabulletMatchPhase::WaitingForPlayers);
			if (NumPlayers < RequiredPlayerCount)
			{
				CancelStartCountdown();
			}
			else
			{
				BeginStartCountdown();
			}
		}
		else if (GetMatchState() == MatchState::InProgress && ExitingPlayerState)
		{
			TurnOrder.Remove(ExitingPlayerState);
			TPGameState->SetTurnOrderPlayerStates(TurnOrder);

			if (RemovedTurnIndex != INDEX_NONE && RemovedTurnIndex < CurrentTurnIndex)
			{
				CurrentTurnIndex--;
			}
			else if (bWasCurrentTurnPlayer)
			{
				CurrentTurnIndex = RemovedTurnIndex - 1;
			}

			if (!UpdateEliminationsAndCheckGameOver() && bWasCurrentTurnPlayer)
			{
				AdvanceTurn();
			}
		}
	}
}

void ATPGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(ETabulletMatchPhase::InGame);
		TPGameState->SetTurnPhase(ETabulletTurnPhase::None);
	}

	InitializeTurnOrder();
	SpawnTablePieces();
	StartFirstTurn();
}

AActor* ATPGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	const APlayerState* JoiningPlayerState = Player ? Player->PlayerState : nullptr;
	const int32 PlayerIndex = TPGameState && JoiningPlayerState
		? TPGameState->PlayerArray.IndexOfByKey(JoiningPlayerState)
		: INDEX_NONE;

	if (PlayerIndex != INDEX_NONE)
	{
		const FName DesiredStartTag(*FString::Printf(TEXT("P%d"), PlayerIndex));
		if (AActor* TaggedPlayerStart = FindPlayerStartByTag(DesiredStartTag, true))
		{
			return TaggedPlayerStart;
		}
	}

	for (int32 StartIndex = 0; StartIndex < RequiredPlayerCount; ++StartIndex)
	{
		const FName FallbackStartTag(*FString::Printf(TEXT("P%d"), StartIndex));
		if (AActor* TaggedPlayerStart = FindPlayerStartByTag(FallbackStartTag, true))
		{
			return TaggedPlayerStart;
		}
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

bool ATPGameMode::IsPlayerStartOccupied(const AActor* PlayerStart) const
{
	if (!PlayerStart || !GetWorld())
	{
		return false;
	}

	const FVector StartLocation = PlayerStart->GetActorLocation();
	constexpr float OccupiedDistanceSquared = 10000.0f;

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		const APawn* Pawn = *It;
		if (Pawn && FVector::DistSquared(Pawn->GetActorLocation(), StartLocation) <= OccupiedDistanceSquared)
		{
			return true;
		}
	}

	return false;
}

AActor* ATPGameMode::FindPlayerStartByTag(FName StartTag, bool bRequireUnoccupied) const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart
			&& (PlayerStart->PlayerStartTag == StartTag || PlayerStart->ActorHasTag(StartTag))
			&& (!bRequireUnoccupied || !IsPlayerStartOccupied(PlayerStart)))
		{
			return PlayerStart;
		}
	}

	return nullptr;
}

bool ATPGameMode::CanStartGame() const
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState || TPGameState->PlayerArray.Num() != RequiredPlayerCount || GetMatchState() != MatchState::WaitingToStart)
	{
		return false;
	}

	return true;
}

void ATPGameMode::StartGame()
{
	if (CanStartGame())
	{
		GetWorldTimerManager().ClearTimer(StartMatchTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("Required players connected. Starting match."));
		StartMatch();
	}
}

void ATPGameMode::BeginStartCountdown()
{
	if (!HasAuthority() || !CanStartGame() || GetWorldTimerManager().IsTimerActive(StartMatchTimerHandle))
	{
		return;
	}

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(ETabulletMatchPhase::Starting);
	}

	UE_LOG(LogTemp, Log, TEXT("Required players connected. Match starts in %.1f seconds."), AutoStartDelay);

	if (AutoStartDelay <= 0.0f)
	{
		StartGame();
		return;
	}

	GetWorldTimerManager().SetTimer(StartMatchTimerHandle, this, &ATPGameMode::StartGame, AutoStartDelay, false);
}

void ATPGameMode::CancelStartCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(StartMatchTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(StartMatchTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("Match start countdown canceled. Waiting for players."));
	}
}

bool ATPGameMode::RequestFlick(AController* RequestingController, AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!HasAuthority() || !TPGameState || TPGameState->MatchPhase != ETabulletMatchPhase::InGame)
	{
		return false;
	}

	if (TPGameState->TurnPhase != ETabulletTurnPhase::WaitingForAction)
	{
		return false;
	}

	if (!IsCurrentTurnController(RequestingController))
	{
		return false;
	}

	if (!IsValid(Table) || !IsValid(Piece) || Piece->IsOut() || !Piece->IsOwnedByPlayerState(RequestingController->PlayerState))
	{
		return false;
	}

	LastFlickPlayerState = RequestingController->PlayerState;

	if (!Table->TryApplyFlick(Piece, WorldDirection, NormalizedPower))
	{
		LastFlickPlayerState = nullptr;
		return false;
	}

	TPGameState->SetTurnPhase(ETabulletTurnPhase::ResolvingPhysics);
	ResolveStartedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	GetWorldTimerManager().SetTimer(ResolveCheckTimerHandle, this, &ATPGameMode::CheckResolveComplete, ResolveCheckInterval, true);

	return true;
}

void ATPGameMode::AdvanceTurn()
{
	if (!HasAuthority() || TurnOrder.IsEmpty())
	{
		return;
	}

	if (UpdateEliminationsAndCheckGameOver())
	{
		return;
	}

	for (int32 Attempt = 0; Attempt < TurnOrder.Num(); ++Attempt)
	{
		const int32 NextTurnIndex = CurrentTurnIndex == INDEX_NONE
			? 0
			: (CurrentTurnIndex + 1) % TurnOrder.Num();

		CurrentTurnIndex = NextTurnIndex;

		const ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(TurnOrder[CurrentTurnIndex]);
		if (TPPlayerState && !TPPlayerState->bIsEliminated)
		{
			SetCurrentTurnByIndex(CurrentTurnIndex);
			return;
		}
	}
}

APlayerState* ATPGameMode::GetCurrentTurnPlayerState() const
{
	return TurnOrder.IsValidIndex(CurrentTurnIndex) ? TurnOrder[CurrentTurnIndex] : nullptr;
}

bool ATPGameMode::IsCurrentTurnController(AController* Controller) const
{
	return Controller && Controller->PlayerState && Controller->PlayerState == GetCurrentTurnPlayerState();
}

void ATPGameMode::AssignPieceOwner(ATableBulletPiece* Piece, APlayerState* NewOwner)
{
	if (!HasAuthority() || !IsValid(Piece))
	{
		return;
	}

	Piece->SetOwningPlayerState(NewOwner);
	RecalculatePlayerPieceCounts();
}

void ATPGameMode::RecalculatePlayerPieceCounts()
{
	if (!HasAuthority())
	{
		return;
	}

	TMap<APlayerState*, int32> PieceCounts;

	if (const ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		for (APlayerState* PlayerState : TPGameState->PlayerArray)
		{
			if (IsValid(PlayerState))
			{
				PieceCounts.Add(PlayerState, 0);
			}
		}
	}

	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<ATableBulletPiece> It(GetWorld()); It; ++It)
	{
		ATableBulletPiece* Piece = *It;
		APlayerState* PieceOwner = Piece ? Piece->GetOwningPlayerState() : nullptr;
		if (IsValid(PieceOwner) && !Piece->IsOut())
		{
			PieceCounts.FindOrAdd(PieceOwner)++;
		}
	}

	for (const TPair<APlayerState*, int32>& PieceCount : PieceCounts)
	{
		if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PieceCount.Key))
		{
			TPPlayerState->SetRemainingPieceCount(PieceCount.Value);
		}
	}
}

void ATPGameMode::InitializeTurnOrder()
{
	TurnOrder.Reset();
	CurrentTurnIndex = INDEX_NONE;

	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState)
	{
		return;
	}

	for (APlayerState* PlayerState : TPGameState->PlayerArray)
	{
		if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState))
		{
			TPPlayerState->SetEliminated(false);
			TurnOrder.Add(TPPlayerState);
		}
	}

	if (ATPGameState* MutableTPGameState = GetGameState<ATPGameState>())
	{
		MutableTPGameState->SetTurnOrderPlayerStates(TurnOrder);
	}

	RecalculatePlayerPieceCounts();
}

void ATPGameMode::SpawnTablePieces()
{
	if (!HasAuthority() || bTablePiecesSpawned)
	{
		return;
	}

	AFlickTableBase* FlickTable = FindFlickTable();
	if (!IsValid(FlickTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn table pieces: FlickTable not found."));
		return;
	}

	FlickTable->OnTablePieceFell.AddUniqueDynamic(this, &ATPGameMode::HandleTablePieceFell);

	TArray<APlayerState*> Players;
	Players.Reserve(TurnOrder.Num());

	for (APlayerState* PlayerState : TurnOrder)
	{
		if (IsValid(PlayerState))
		{
			Players.Add(PlayerState);
		}
	}

	const int32 SpawnedNormalPieces = FlickTable->SpawnNormalPiecesForPlayers(Players, PiecesPerPlayer);
	const int32 SpawnedSpecialPieces = FlickTable->SpawnSpecialPieces(SpecialPieceCount);

	bTablePiecesSpawned = SpawnedNormalPieces > 0 || SpawnedSpecialPieces > 0;

	UE_LOG(LogTemp, Log, TEXT("Spawned table pieces. Normal=%d Special=%d"), SpawnedNormalPieces, SpawnedSpecialPieces);

	RecalculatePlayerPieceCounts();
}

AFlickTableBase* ATPGameMode::FindFlickTable() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	for (TActorIterator<AFlickTableBase> It(GetWorld()); It; ++It)
	{
		if (AFlickTableBase* FlickTable = *It; IsValid(FlickTable))
		{
			return FlickTable;
		}
	}

	return nullptr;
}

void ATPGameMode::StartFirstTurn()
{
	if (!HasAuthority() || TurnOrder.IsEmpty())
	{
		return;
	}

	SetCurrentTurnByIndex(0);
}

void ATPGameMode::SetCurrentTurnByIndex(int32 NewTurnIndex)
{
	if (!HasAuthority() || !TurnOrder.IsValidIndex(NewTurnIndex))
	{
		return;
	}

	CurrentTurnIndex = NewTurnIndex;

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetCurrentTurnPlayerState(GetCurrentTurnPlayerState(), TPGameState->TurnNumber + 1);
		TPGameState->SetTurnPhase(ETabulletTurnPhase::WaitingForAction);
	}
}

void ATPGameMode::CheckResolveComplete()
{
	const bool bTimedOut = GetWorld() && GetWorld()->GetTimeSeconds() - ResolveStartedTime >= MaxResolveSeconds;
	if (!bTimedOut && AreAnyPiecesMoving())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(ResolveCheckTimerHandle);

	if (UpdateEliminationsAndCheckGameOver())
	{
		return;
	}

	AdvanceTurn();
}

bool ATPGameMode::AreAnyPiecesMoving() const
{
	if (!GetWorld())
	{
		return false;
	}

	for (TActorIterator<ATableBulletPiece> It(GetWorld()); It; ++It)
	{
		if (const ATableBulletPiece* Piece = *It; Piece && Piece->IsMoving(PieceStoppedSpeedThreshold, PieceStoppedSpeedThreshold))
		{
			return true;
		}
	}

	return false;
}

bool ATPGameMode::UpdateEliminationsAndCheckGameOver()
{
	RecalculatePlayerPieceCounts();

	bool bHasAnyOwnedPiece = false;
	int32 ActivePlayerCount = 0;
	APlayerState* LastActivePlayerState = nullptr;

	for (APlayerState* PlayerState : TurnOrder)
	{
		ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState);
		if (!TPPlayerState)
		{
			continue;
		}

		if (TPPlayerState->RemainingPieceCount > 0)
		{
			bHasAnyOwnedPiece = true;
			TPPlayerState->SetEliminated(false);
			ActivePlayerCount++;
			LastActivePlayerState = TPPlayerState;
		}
		else if (bHasAnyOwnedPiece)
		{
			TPPlayerState->SetEliminated(true);
		}
	}

	if (!bHasAnyOwnedPiece)
	{
		return false;
	}

	for (APlayerState* PlayerState : TurnOrder)
	{
		if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState))
		{
			TPPlayerState->SetEliminated(TPPlayerState->RemainingPieceCount <= 0);
		}
	}

	if (ActivePlayerCount <= 1)
	{
		StartShootingPhase(LastActivePlayerState);
		return true;
	}

	return false;
}

void ATPGameMode::HandleTablePieceFell(ATableBulletPiece* FallenPiece, APlayerState* PieceOwner)
{
	AwardAmmoForFallenPiece(FallenPiece, PieceOwner);
}

void ATPGameMode::AwardAmmoForFallenPiece(ATableBulletPiece* FallenPiece, APlayerState* PieceOwner)
{
	APlayerState* CapturingPlayer = LastFlickPlayerState;

	if (!HasAuthority() || !IsValid(FallenPiece) || !IsValid(CapturingPlayer) || CapturingPlayer == PieceOwner)
	{
		return;
	}

	ATPCharacter* CapturingCharacter = GetCharacterForPlayerState(CapturingPlayer);
	UAmmoComponent* AmmoComponent = CapturingCharacter ? CapturingCharacter->FindComponentByClass<UAmmoComponent>() : nullptr;
	if (!AmmoComponent)
	{
		return;
	}

	const EWeaponType AmmoType = FallenPiece->GetPieceType() == ETablePieceType::Special
		? FallenPiece->GetRewardWeaponType()
		: EWeaponType::Revolver;

	AmmoComponent->SetAmmoCount(AmmoType, AmmoComponent->GetAmmoCount(AmmoType) + 1);
	UE_LOG(LogTemp, Log, TEXT("Awarded ammo. Player=%s Type=%d"), *CapturingPlayer->GetPlayerName(), static_cast<int32>(AmmoType));
}

void ATPGameMode::StartShootingPhase(APlayerState* TableWinner)
{
	if (!HasAuthority())
	{
		return;
	}

	TablePhaseWinner = TableWinner;
	BuildShootingTurnOrder();

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetWinnerPlayerState(TableWinner);
		TPGameState->SetMatchPhase(ETabulletMatchPhase::ShootingPhase);
		TPGameState->SetTurnOrderPlayerStates(ShootingTurnOrder);
	}

	CurrentShootingTurnIndex = INDEX_NONE;
	AdvanceShootingTurn();
}

void ATPGameMode::BuildShootingTurnOrder()
{
	ShootingTurnOrder.Reset();

	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState)
	{
		return;
	}

	for (APlayerState* PlayerState : TPGameState->PlayerArray)
	{
		if (IsValid(PlayerState) && IsPlayerAlive(PlayerState) && HasAnyAmmo(PlayerState))
		{
			ShootingTurnOrder.Add(PlayerState);
		}
	}

	ShootingTurnOrder.Sort([this](const TObjectPtr<APlayerState>& Left, const TObjectPtr<APlayerState>& Right)
	{
		const int32 LeftAmmo = GetTotalAmmoCount(Left.Get());
		const int32 RightAmmo = GetTotalAmmoCount(Right.Get());
		if (LeftAmmo != RightAmmo)
		{
			return LeftAmmo > RightAmmo;
		}

		const ATPPlayerState* LeftTP = Cast<ATPPlayerState>(Left.Get());
		const ATPPlayerState* RightTP = Cast<ATPPlayerState>(Right.Get());
		const int32 LeftIndex = LeftTP ? LeftTP->PlayerIndex : MAX_int32;
		const int32 RightIndex = RightTP ? RightTP->PlayerIndex : MAX_int32;
		return LeftIndex < RightIndex;
	});
}

void ATPGameMode::AdvanceShootingTurn()
{
	if (!HasAuthority())
	{
		return;
	}

	if (CheckShootingGameOver())
	{
		return;
	}

	for (int32 Attempt = 0; Attempt < ShootingTurnOrder.Num(); ++Attempt)
	{
		CurrentShootingTurnIndex = CurrentShootingTurnIndex == INDEX_NONE
			? 0
			: (CurrentShootingTurnIndex + 1) % ShootingTurnOrder.Num();

		APlayerState* Candidate = ShootingTurnOrder[CurrentShootingTurnIndex];
		if (IsValid(Candidate) && IsPlayerAlive(Candidate) && HasAnyAmmo(Candidate))
		{
			if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
			{
				TPGameState->SetCurrentTurnPlayerState(Candidate, TPGameState->TurnNumber + 1);
				TPGameState->SetTurnPhase(ETabulletTurnPhase::WaitingForShot);
			}
			return;
		}
	}

	FinishGame(TablePhaseWinner);
}

void ATPGameMode::NotifyShotResolved(AController* ShootingController)
{
	ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!HasAuthority() || !TPGameState || TPGameState->MatchPhase != ETabulletMatchPhase::ShootingPhase)
	{
		return;
	}

	if (!IsCurrentTurnController(ShootingController))
	{
		return;
	}

	AdvanceShootingTurn();
}

bool ATPGameMode::CheckShootingGameOver()
{
	int32 AlivePlayerCount = 0;
	APlayerState* LastAlivePlayerState = nullptr;

	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState)
	{
		return false;
	}

	for (APlayerState* PlayerState : TPGameState->PlayerArray)
	{
		if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState))
		{
			TPPlayerState->SetEliminated(!IsPlayerAlive(PlayerState));
		}

		if (IsPlayerAlive(PlayerState))
		{
			AlivePlayerCount++;
			LastAlivePlayerState = PlayerState;
		}
	}

	if (AlivePlayerCount <= 1)
	{
		FinishGame(LastAlivePlayerState);
		return true;
	}

	return false;
}

bool ATPGameMode::IsPlayerAlive(APlayerState* PlayerState) const
{
	const ATPCharacter* Character = GetCharacterForPlayerState(PlayerState);
	const UHealthComponent* HealthComponent = Character ? Character->FindComponentByClass<UHealthComponent>() : nullptr;
	return HealthComponent && !HealthComponent->IsDead();
}

bool ATPGameMode::HasAnyAmmo(APlayerState* PlayerState) const
{
	return GetTotalAmmoCount(PlayerState) > 0;
}

int32 ATPGameMode::GetTotalAmmoCount(APlayerState* PlayerState) const
{
	const ATPCharacter* Character = GetCharacterForPlayerState(PlayerState);
	const UAmmoComponent* AmmoComponent = Character ? Character->FindComponentByClass<UAmmoComponent>() : nullptr;
	return AmmoComponent
		? AmmoComponent->GetAmmoCount(EWeaponType::Revolver)
			+ AmmoComponent->GetAmmoCount(EWeaponType::Shotgun)
			+ AmmoComponent->GetAmmoCount(EWeaponType::Sniper)
		: 0;
}

ATPCharacter* ATPGameMode::GetCharacterForPlayerState(APlayerState* PlayerState) const
{
	if (!IsValid(PlayerState) || !GetWorld())
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (!PlayerController || PlayerController->PlayerState != PlayerState)
		{
			continue;
		}

		return Cast<ATPCharacter>(PlayerController->GetPawn());
	}

	return nullptr;
}

void ATPGameMode::FinishGame(APlayerState* Winner)
{
	GetWorldTimerManager().ClearTimer(ResolveCheckTimerHandle);

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetWinnerPlayerState(Winner);
		TPGameState->SetCurrentTurnPlayerState(nullptr, TPGameState->TurnNumber);
		TPGameState->SetTurnPhase(ETabulletTurnPhase::None);
		TPGameState->SetMatchPhase(ETabulletMatchPhase::GameOver);
	}

	if (GetMatchState() == MatchState::InProgress)
	{
		EndMatch();
	}
}

