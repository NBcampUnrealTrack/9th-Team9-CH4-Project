// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "LobbyHUD.h"
#include "TPGameInstance.h"
#include "TPGameState.h"
#include "TPPlayerController.h"

ALobbyGameMode::ALobbyGameMode()
{
	bDelayedStart = true;
	bUseSeamlessTravel = true;
	GameStateClass = ATPGameState::StaticClass();
	PlayerControllerClass = ATPPlayerController::StaticClass();
	PlayerStateClass = ATPPlayerState::StaticClass();
	DefaultPawnClass = nullptr;
	HUDClass = ALobbyHUD::StaticClass();
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UTPGameInstance* TPGameInstance = GetGameInstance<UTPGameInstance>())
	{
		TPGameInstance->ClearLobbyCharacterSelections();
	}
}

void ALobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (bLobbyLocked)
	{
		ErrorMessage = TEXT("The lobby is starting.");
		return;
	}

	if (NumPlayers >= RequiredPlayerCount)
	{
		ErrorMessage = TEXT("The lobby is full.");
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ATPPlayerState* TPPlayerState = NewPlayer ? NewPlayer->GetPlayerState<ATPPlayerState>() : nullptr)
	{
		TPPlayerState->SetReady(false);
		TPPlayerState->SetSelectedCharacterType(ETPCharacterType::None);
	}

	if (ATPPlayerController* TPPlayerController = Cast<ATPPlayerController>(NewPlayer))
	{
		TPPlayerController->SetSelectedLobbyCharacterType(ETPCharacterType::None);
	}

	RefreshPlayerIndices();
	RefreshLobbyMatchPhase();
	CheckStartConditions();
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!bLobbyLocked)
	{
		GetWorldTimerManager().ClearTimer(StartTravelTimerHandle);
	}

	RefreshPlayerIndices();
	RefreshLobbyMatchPhase();
	CheckStartConditions();
}

bool ALobbyGameMode::RequestSetPlayerReady(APlayerController* PlayerController, bool bReady)
{
	if (!HasAuthority() || bLobbyLocked)
	{
		return false;
	}

	ATPPlayerState* TPPlayerState = PlayerController ? PlayerController->GetPlayerState<ATPPlayerState>() : nullptr;
	if (!TPPlayerState)
	{
		return false;
	}

	if (bReady && bRequireCharacterSelection && TPPlayerState->SelectedCharacterType == ETPCharacterType::None)
	{
		return false;
	}

	if (bReady && !IsCharacterAvailable(TPPlayerState->SelectedCharacterType, TPPlayerState))
	{
		return false;
	}

	TPPlayerState->SetReady(bReady);
	RefreshLobbyMatchPhase();
	CheckStartConditions();
	return true;
}

bool ALobbyGameMode::RequestSelectCharacter(APlayerController* PlayerController, ETPCharacterType CharacterType)
{
	if (!HasAuthority() || bLobbyLocked || CharacterType == ETPCharacterType::None)
	{
		return false;
	}

	ATPPlayerState* TPPlayerState = PlayerController ? PlayerController->GetPlayerState<ATPPlayerState>() : nullptr;
	if (!TPPlayerState || !IsCharacterAvailable(CharacterType, TPPlayerState))
	{
		return false;
	}

	TPPlayerState->SetSelectedCharacterType(CharacterType);
	TPPlayerState->SetReady(false);
	if (ATPPlayerController* TPPlayerController = Cast<ATPPlayerController>(PlayerController))
	{
		TPPlayerController->SetSelectedLobbyCharacterType(CharacterType);
	}
	if (UTPGameInstance* TPGameInstance = GetGameInstance<UTPGameInstance>())
	{
		TPGameInstance->SetLobbyCharacterSelection(TPPlayerState->PlayerIndex, CharacterType);
	}
	RefreshLobbyMatchPhase();
	return true;
}

bool ALobbyGameMode::IsLobbyReadyToStart() const
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState || TPGameState->PlayerArray.Num() != RequiredPlayerCount)
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

		if (bRequireCharacterSelection && TPPlayerState->SelectedCharacterType == ETPCharacterType::None)
		{
			return false;
		}
	}

	return true;
}

int32 ALobbyGameMode::GetConnectedPlayerCount() const
{
	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	return TPGameState ? TPGameState->PlayerArray.Num() : 0;
}

void ALobbyGameMode::RefreshPlayerIndices()
{
	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		for (int32 PlayerIndex = 0; PlayerIndex < TPGameState->PlayerArray.Num(); ++PlayerIndex)
		{
			if (ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(TPGameState->PlayerArray[PlayerIndex]))
			{
				TPPlayerState->SetPlayerIndex(PlayerIndex);
			}
		}
	}
}

void ALobbyGameMode::RefreshLobbyMatchPhase()
{
	if (ATPGameState* TPGameState = GetGameState<ATPGameState>())
	{
		TPGameState->SetMatchPhase(IsLobbyReadyToStart()
			? ETabulletMatchPhase::Starting
			: ETabulletMatchPhase::WaitingForPlayers);
	}
}

void ALobbyGameMode::CheckStartConditions()
{
	if (!HasAuthority() || bLobbyLocked)
	{
		return;
	}

	if (!IsLobbyReadyToStart())
	{
		GetWorldTimerManager().ClearTimer(StartTravelTimerHandle);
		return;
	}

	bLobbyLocked = true;
	GetWorldTimerManager().SetTimer(StartTravelTimerHandle, this, &ALobbyGameMode::TravelToInGameMap, StartTravelDelay, false);
}

void ALobbyGameMode::TravelToInGameMap()
{
	if (!HasAuthority() || InGameMapPath.IsEmpty() || !GetWorld())
	{
		return;
	}

	const FString TravelURL = InGameMapPath;
	GetWorld()->ServerTravel(TravelURL);
}

bool ALobbyGameMode::IsCharacterAvailable(ETPCharacterType CharacterType, const ATPPlayerState* RequestingPlayerState) const
{
	if (bAllowDuplicateCharacters || CharacterType == ETPCharacterType::None)
	{
		return true;
	}

	const ATPGameState* TPGameState = GetGameState<ATPGameState>();
	if (!TPGameState)
	{
		return true;
	}

	for (APlayerState* PlayerState : TPGameState->PlayerArray)
	{
		const ATPPlayerState* TPPlayerState = Cast<ATPPlayerState>(PlayerState);
		if (TPPlayerState && TPPlayerState != RequestingPlayerState && TPPlayerState->SelectedCharacterType == CharacterType)
		{
			return false;
		}
	}

	return true;
}
