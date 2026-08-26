// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameState.h"

#include "Net/UnrealNetwork.h"

ATPGameState::ATPGameState()
{
	MatchPhase = ETabulletMatchPhase::WaitingForPlayers;
}

void ATPGameState::SetMatchPhase(ETabulletMatchPhase NewPhase)
{
	if (HasAuthority())
	{
		MatchPhase = NewPhase;
	}
}

void ATPGameState::OnRep_MatchPhase()
{
}

void ATPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPGameState, MatchPhase);
}

