// Interaction.cpp

#include "InteractionComponent.h"
#include "GameFramework/Pawn.h"
#include "TabulletProject/TPGameState.h"
#include "TabulletProject/TPPlayerState.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

const ATPGameState* UInteractionComponent::GetTPGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ATPGameState>() : nullptr;
}

const ATPPlayerState* UInteractionComponent::GetOwnerPlayerState() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return  OwnerPawn ? OwnerPawn->GetPlayerState<ATPPlayerState>() : nullptr;
}

bool UInteractionComponent::IsMyTurn() const
{
	const ATPGameState* TPGameState = GetTPGameState();
	const ATPPlayerState* MyPlayerState = GetOwnerPlayerState();
	
	if (!TPGameState || !MyPlayerState)
	{
		return false;
	}

	if (TPGameState->MatchPhase == ETabulletMatchPhase::InGame && MyPlayerState->bIsTableEliminated)
	{
		return false;
	}

	if (TPGameState->MatchPhase == ETabulletMatchPhase::ShootingPhase && MyPlayerState->bIsEliminated)
	{
		return false;
	}
	
	return TPGameState->CurrentTurnPlayerState == MyPlayerState;
}

bool UInteractionComponent::CanFlick() const
{
	const ATPGameState* TPGameState = GetTPGameState();
	const ATPPlayerState* MyPlayerState = GetOwnerPlayerState();
	
	if (!TPGameState || !MyPlayerState || MyPlayerState->bIsTableEliminated)
	{
		return false;
	}
	
	return TPGameState->MatchPhase == ETabulletMatchPhase::InGame
		&& TPGameState->TurnPhase == ETabulletTurnPhase::WaitingForAction
		&& TPGameState->CurrentTurnPlayerState == MyPlayerState;
}

bool UInteractionComponent::CanShoot() const
{
	const ATPGameState* TPGameState = GetTPGameState();
	const ATPPlayerState* MyPlayerState = GetOwnerPlayerState();
	
	if (!TPGameState || !MyPlayerState || MyPlayerState->bIsEliminated)
	{
		return false;
	}
	
	return TPGameState->MatchPhase == ETabulletMatchPhase::ShootingPhase
		&& TPGameState->TurnPhase == ETabulletTurnPhase::WaitingForShot
		&& TPGameState->CurrentTurnPlayerState == MyPlayerState;
}

