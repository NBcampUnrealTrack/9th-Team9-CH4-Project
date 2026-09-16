// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPPlayerState.generated.h"

UENUM(BlueprintType)
enum class ETPCharacterType : uint8
{
	None,
	Dog,
	Fox,
	Bull,
	Raccoon
};

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void CopyProperties(APlayerState* PlayerState) override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, BlueprintReadOnly, Category = "Player")
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingPieceCount, BlueprintReadOnly, Category = "Table")
	int32 RemainingPieceCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated, BlueprintReadOnly, Category = "Match")
	bool bIsEliminated = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsTableEliminated, BlueprintReadOnly, Category = "Table")
	bool bIsTableEliminated = false;

	UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
	bool bIsReady = false;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedCharacterType, BlueprintReadOnly, Category = "Lobby")
	ETPCharacterType SelectedCharacterType = ETPCharacterType::None;
	
	void SetPlayerIndex(int32 NewPlayerIndex);
	void SetRemainingPieceCount(int32 NewRemainingPieceCount);
	void SetEliminated(bool bNewIsEliminated);
	void SetTableEliminated(bool bNewIsTableEliminated);
	void SetReady(bool bNewIsReady);
	void SetSelectedCharacterType(ETPCharacterType NewSelectedCharacterType);

	UFUNCTION()
	void OnRep_PlayerIndex();

	UFUNCTION()
	void OnRep_RemainingPieceCount();

	UFUNCTION()
	void OnRep_IsEliminated();

	UFUNCTION()
	void OnRep_IsTableEliminated();

	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_SelectedCharacterType();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player")
	void OnPlayerIndexChanged(int32 NewPlayerIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Table")
	void OnRemainingPieceCountChanged(int32 NewRemainingPieceCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnEliminatedChanged(bool bNewIsEliminated);

	UFUNCTION(BlueprintImplementableEvent, Category = "Table")
	void OnTableEliminatedChanged(bool bNewIsTableEliminated);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void OnReadyChanged(bool bNewIsReady);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void OnSelectedCharacterTypeChanged(ETPCharacterType NewSelectedCharacterType);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
