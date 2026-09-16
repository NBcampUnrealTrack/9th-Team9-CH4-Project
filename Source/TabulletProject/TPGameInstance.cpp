// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameInstance.h"

void UTPGameInstance::ClearLobbyCharacterSelections()
{
	LobbyCharacterSelectionsByPlayerIndex.Reset();
}

void UTPGameInstance::SetLobbyCharacterSelection(int32 PlayerIndex, ETPCharacterType CharacterType)
{
	if (PlayerIndex == INDEX_NONE || CharacterType == ETPCharacterType::None)
	{
		return;
	}

	LobbyCharacterSelectionsByPlayerIndex.Add(PlayerIndex, CharacterType);
}

ETPCharacterType UTPGameInstance::GetLobbyCharacterSelection(int32 PlayerIndex) const
{
	if (const ETPCharacterType* CharacterType = LobbyCharacterSelectionsByPlayerIndex.Find(PlayerIndex))
	{
		return *CharacterType;
	}

	return ETPCharacterType::None;
}
