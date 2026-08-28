#include "TPPlayerController.h"

#include "TPGameMode.h"

void ATPPlayerController::ServerSetReady_Implementation(bool bReady)
{
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->SetPlayerReady(this, bReady);
	}
}

