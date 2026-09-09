// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameState.h"

#include "Net/UnrealNetwork.h"

ATPGameState::ATPGameState()
{
	MatchPhase = ETabulletMatchPhase::WaitingForPlayers;
}

void ATPGameState::SetMatchPhase(ETabulletMatchPhase NewPhase)
{
	if (HasAuthority() && MatchPhase != NewPhase)
	{
		MatchPhase = NewPhase;
		OnMatchPhaseChanged(MatchPhase);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

void ATPGameState::SetCurrentTurnPlayerState(APlayerState* NewTurnPlayerState, int32 NewTurnNumber)
{
	if (HasAuthority() && (CurrentTurnPlayerState != NewTurnPlayerState || TurnNumber != NewTurnNumber))
	{
		CurrentTurnPlayerState = NewTurnPlayerState;
		TurnNumber = NewTurnNumber;
		OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

void ATPGameState::SetTurnPhase(ETabulletTurnPhase NewTurnPhase)
{
	if (HasAuthority() && TurnPhase != NewTurnPhase)
	{
		TurnPhase = NewTurnPhase;
		OnTurnPhaseChanged(TurnPhase);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

void ATPGameState::SetTurnOrderPlayerStates(const TArray<TObjectPtr<APlayerState>>& NewTurnOrderPlayerStates)
{
	if (HasAuthority())
	{
		TurnOrderPlayerStates = NewTurnOrderPlayerStates;
		OnTurnOrderChanged();
	}
}

void ATPGameState::SetWinnerPlayerState(APlayerState* NewWinnerPlayerState)
{
	if (HasAuthority() && WinnerPlayerState != NewWinnerPlayerState)
	{
		WinnerPlayerState = NewWinnerPlayerState;
		OnWinnerChanged(WinnerPlayerState);
	}
}

void ATPGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged(MatchPhase);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_CurrentTurnPlayerState()
{
	OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnNumber()
{
	OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnPhase()
{
	OnTurnPhaseChanged(TurnPhase);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnOrderPlayerStates()
{
	OnTurnOrderChanged();
}

void ATPGameState::OnRep_WinnerPlayerState()
{
	OnWinnerChanged(WinnerPlayerState);
}

void ATPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPGameState, MatchPhase);
	DOREPLIFETIME(ATPGameState, CurrentTurnPlayerState);
	DOREPLIFETIME(ATPGameState, TurnNumber);
	DOREPLIFETIME(ATPGameState, TurnPhase);
	DOREPLIFETIME(ATPGameState, TurnOrderPlayerStates);
	DOREPLIFETIME(ATPGameState, WinnerPlayerState);
}

