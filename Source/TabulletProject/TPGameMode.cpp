// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameMode.h"

#include "TPGameState.h"
#include "TPPlayerController.h"
#include "TPPlayerState.h"
#include "EngineUtils.h"
#include "TabulletProject/Table/Actors/FlickTableBase.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"

ATPGameMode::ATPGameMode()
{
	GameStateClass = ATPGameState::StaticClass();
	PlayerControllerClass = ATPPlayerController::StaticClass();
	PlayerStateClass = ATPPlayerState::StaticClass();
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

		TPGameState->SetMatchPhase(NumPlayers >= RequiredPlayerCount ? ETabulletMatchPhase::ReadyCheck : ETabulletMatchPhase::WaitingForPlayers);
	}

	StartGame();
}

void ATPGameMode::Logout(AController* Exiting)
{
	APlayerState* ExitingPlayerState = Exiting ? Exiting->PlayerState : nullptr;
	const bool bWasCurrentTurnPlayer = ExitingPlayerState && ExitingPlayerState == GetCurrentTurnPlayerState();
	const int32 RemovedTurnIndex = ExitingPlayerState ? TurnOrder.IndexOfByKey(ExitingPlayerState) : INDEX_NONE;

	if (ATPPlayerState* TPPlayerState = Exiting ? Exiting->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetReady(false);
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

			TPGameState->SetMatchPhase(NumPlayers >= RequiredPlayerCount ? ETabulletMatchPhase::ReadyCheck : ETabulletMatchPhase::WaitingForPlayers);
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
	StartFirstTurn();
}

void ATPGameMode::SetPlayerReady(AController* Player, bool bReady)
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState || TPGameState->MatchPhase != ETabulletMatchPhase::ReadyCheck)
	{
		return;
	}

	if (ATPPlayerState* TPPlayerState = Player ? Player->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetReady(bReady);
	}

	StartGame();
}

bool ATPGameMode::CanStartGame() const
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState || TPGameState->PlayerArray.Num() != RequiredPlayerCount || GetMatchState() != MatchState::WaitingToStart)
	{
		return false;
	}

	for (APlayerState* PlayerState : TPGameState->PlayerArray)
	{
		const ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState);
		if (!TPPlayerState || !TPPlayerState->bIsReady)
		{
			return false;
		}
	}

	return true;
}

void ATPGameMode::StartGame()
{
	if (CanStartGame())
	{
		StartMatch();
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

	if (!IsValid(Table) || !IsValid(Piece) || Piece->IsOut() || !Piece->IsOwnedBy(RequestingController->PlayerState))
	{
		return false;
	}

	if (!Table->TryApplyFlick(Piece, WorldDirection, NormalizedPower))
	{
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
		APlayerState* Owner = Piece ? Piece->GetOwningPlayerState() : nullptr;
		if (IsValid(Owner) && !Piece->IsOut())
		{
			PieceCounts.FindOrAdd(Owner)++;
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
		if (const ATableBulletPiece* Piece = *It; Piece && Piece->IsMovingAboveSpeed(PieceStoppedSpeedThreshold))
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
		FinishGame(LastActivePlayerState);
		return true;
	}

	return false;
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

