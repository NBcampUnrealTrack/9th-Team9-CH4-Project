// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TPPlayerState.h"
#include "TPGameInstance.generated.h"

UCLASS()
class TABULLETPROJECT_API UTPGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void ClearLobbyCharacterSelections();
	void SetLobbyCharacterSelection(int32 PlayerIndex, ETPCharacterType CharacterType);
	ETPCharacterType GetLobbyCharacterSelection(int32 PlayerIndex) const;

private:
	UPROPERTY()
	TMap<int32, ETPCharacterType> LobbyCharacterSelectionsByPlayerIndex;
};
