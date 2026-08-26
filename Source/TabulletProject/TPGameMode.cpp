// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameMode.h"

#include "TPGameState.h"
#include "TPPlayerState.h"

ATPGameMode::ATPGameMode()
{
	GameStateClass = ATPGameState::StaticClass();
	PlayerStateClass = ATPPlayerState::StaticClass();
}

void ATPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(NumPlayers >= 4 ? ETabulletMatchPhase::ReadyCheck : ETabulletMatchPhase::WaitingForPlayers);
	}

	StartGame();
}

void ATPGameMode::Logout(AController* Exiting)
{
	if (ATPPlayerState* TPPlayerState = Exiting ? Exiting->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetReady(false);
	}

	Super::Logout(Exiting);

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(ETabulletMatchPhase::WaitingForPlayers);
	}
}

void ATPGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(ETabulletMatchPhase::InGame);
	}
}

void ATPGameMode::SetPlayerReady(AController* Player, bool bReady)
{
	if (ATPPlayerState* TPPlayerState = Player ? Player->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetReady(bReady);
	}

	StartGame();
}

bool ATPGameMode::CanStartGame() const
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState || TPGameState->PlayerArray.Num() != 4 || GetMatchState() != MatchState::WaitingToStart)
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

