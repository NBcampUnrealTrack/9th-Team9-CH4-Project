#include "TPPlayerController.h"

#include "TPGameMode.h"
#include "TPGameState.h"

void ATPPlayerController::ServerSetReady_Implementation(bool bReady)
{
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->SetPlayerReady(this, bReady);
	}
}

void ATPPlayerController::ServerRequestFlick_Implementation(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->RequestFlick(this, Table, Piece, WorldDirection, NormalizedPower);
	}
}

bool ATPPlayerController::IsMyTurn() const
{
	if (const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr)
	{
		return PlayerState && TPGameState->CurrentTurnPlayerState == PlayerState;
	}

	return false;
}

