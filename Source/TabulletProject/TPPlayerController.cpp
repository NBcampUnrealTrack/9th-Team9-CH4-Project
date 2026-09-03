#include "TPPlayerController.h"

#include "TPGameMode.h"
#include "TPGameState.h"
#include "TPPlayerState.h"

void ATPPlayerController::ServerRequestFlick_Implementation(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->RequestFlick(this, Table, Piece, WorldDirection, NormalizedPower);
	}
}

bool ATPPlayerController::IsMyTurn() const
{
	return PlayerState && GetCurrentTurnPlayerState() == PlayerState;
}

APlayerState* ATPPlayerController::GetCurrentTurnPlayerState() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	return TPGameState ? TPGameState->CurrentTurnPlayerState.Get() : nullptr;
}

int32 ATPPlayerController::GetCurrentTurnPlayerIndex() const
{
	const ATPPlayerState* CurrentTurnPlayerState = Cast<ATPPlayerState>(GetCurrentTurnPlayerState());
	return CurrentTurnPlayerState ? CurrentTurnPlayerState->PlayerIndex : INDEX_NONE;
}

FText ATPPlayerController::GetCurrentTurnText() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState || TPGameState->MatchPhase != ETabulletMatchPhase::InGame || !TPGameState->CurrentTurnPlayerState)
	{
		return FText::GetEmpty();
	}

	if (IsMyTurn())
	{
		return FText::Format(NSLOCTEXT("TPPlayerController", "YourTurnFormat", "Turn {0} - Your Turn"), TPGameState->TurnNumber);
	}

	const int32 CurrentTurnPlayerIndex = GetCurrentTurnPlayerIndex();
	if (CurrentTurnPlayerIndex == INDEX_NONE)
	{
		return FText::Format(NSLOCTEXT("TPPlayerController", "TurnWaitingFormat", "Turn {0}"), TPGameState->TurnNumber);
	}

	return FText::Format(
		NSLOCTEXT("TPPlayerController", "OtherPlayerTurnFormat", "Turn {0} - Player {1} Turn"),
		TPGameState->TurnNumber,
		CurrentTurnPlayerIndex + 1);
}

