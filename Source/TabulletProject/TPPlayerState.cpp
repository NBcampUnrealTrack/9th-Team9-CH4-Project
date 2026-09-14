// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerState.h"

#include "Net/UnrealNetwork.h"

void ATPPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (ATPPlayerState* NewTPPlayerState = Cast<ATPPlayerState>(PlayerState))
	{
		NewTPPlayerState->PlayerIndex = PlayerIndex;
		NewTPPlayerState->bIsReady = bIsReady;
		NewTPPlayerState->SelectedCharacterType = SelectedCharacterType;
	}
}

void ATPPlayerState::SetPlayerIndex(int32 NewPlayerIndex)
{
	if (HasAuthority() && PlayerIndex != NewPlayerIndex)
	{
		PlayerIndex = NewPlayerIndex;
		OnPlayerIndexChanged(PlayerIndex);
	}
}

void ATPPlayerState::SetRemainingPieceCount(int32 NewRemainingPieceCount)
{
	if (HasAuthority() && RemainingPieceCount != NewRemainingPieceCount)
	{
		RemainingPieceCount = NewRemainingPieceCount;
		OnRemainingPieceCountChanged(RemainingPieceCount);
	}
}

void ATPPlayerState::SetEliminated(bool bNewIsEliminated)
{
	if (HasAuthority() && bIsEliminated != bNewIsEliminated)
	{
		bIsEliminated = bNewIsEliminated;
		OnEliminatedChanged(bIsEliminated);
	}
}

void ATPPlayerState::SetTableEliminated(bool bNewIsTableEliminated)
{
	if (HasAuthority() && bIsTableEliminated != bNewIsTableEliminated)
	{
		bIsTableEliminated = bNewIsTableEliminated;
		OnTableEliminatedChanged(bIsTableEliminated);
	}
}

void ATPPlayerState::SetReady(bool bNewIsReady)
{
	if (HasAuthority() && bIsReady != bNewIsReady)
	{
		bIsReady = bNewIsReady;
		OnReadyChanged(bIsReady);
	}
}

void ATPPlayerState::SetSelectedCharacterType(ETPCharacterType NewSelectedCharacterType)
{
	if (HasAuthority() && SelectedCharacterType != NewSelectedCharacterType)
	{
		SelectedCharacterType = NewSelectedCharacterType;
		OnSelectedCharacterTypeChanged(SelectedCharacterType);
	}
}

void ATPPlayerState::OnRep_PlayerIndex()
{
	OnPlayerIndexChanged(PlayerIndex);
}

void ATPPlayerState::OnRep_RemainingPieceCount()
{
	OnRemainingPieceCountChanged(RemainingPieceCount);
}

void ATPPlayerState::OnRep_IsEliminated()
{
	OnEliminatedChanged(bIsEliminated);
}

void ATPPlayerState::OnRep_IsTableEliminated()
{
	OnTableEliminatedChanged(bIsTableEliminated);
}

void ATPPlayerState::OnRep_IsReady()
{
	OnReadyChanged(bIsReady);
}

void ATPPlayerState::OnRep_SelectedCharacterType()
{
	OnSelectedCharacterTypeChanged(SelectedCharacterType);
}

void ATPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerState, PlayerIndex);
	DOREPLIFETIME(ATPPlayerState, RemainingPieceCount);
	DOREPLIFETIME(ATPPlayerState, bIsEliminated);
	DOREPLIFETIME(ATPPlayerState, bIsTableEliminated);
	DOREPLIFETIME(ATPPlayerState, bIsReady);
	DOREPLIFETIME(ATPPlayerState, SelectedCharacterType);
}

