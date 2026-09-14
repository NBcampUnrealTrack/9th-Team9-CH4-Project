// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/OnlineReplStructs.h"
#include "TPPlayerState.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	bool RequestSetPlayerReady(APlayerController* PlayerController, bool bReady);
	bool RequestSelectCharacter(APlayerController* PlayerController, ETPCharacterType CharacterType);

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsLobbyReadyToStart() const;

	UFUNCTION(BlueprintPure, Category = "Lobby")
	int32 GetConnectedPlayerCount() const;

protected:
	void RefreshPlayerIndices();
	void RefreshLobbyMatchPhase();
	void CheckStartConditions();
	void TravelToInGameMap();
	bool IsCharacterAvailable(ETPCharacterType CharacterType, const ATPPlayerState* RequestingPlayerState) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby", meta = (ClampMin = "1"))
	int32 RequiredPlayerCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	FString InGameMapPath = TEXT("/Game/Tabullet/Maps/Prototype");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby", meta = (ClampMin = "0.0"))
	float StartTravelDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	bool bRequireCharacterSelection = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	bool bAllowDuplicateCharacters = true;

	FTimerHandle StartTravelTimerHandle;
	bool bLobbyLocked = false;
};
