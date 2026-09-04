// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerIndex, BlueprintReadOnly, Category = "Player")
	int32 PlayerIndex = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingPieceCount, BlueprintReadOnly, Category = "Table")
	int32 RemainingPieceCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_IsEliminated, BlueprintReadOnly, Category = "Match")
	bool bIsEliminated = false;
	
	void SetPlayerIndex(int32 NewPlayerIndex);
	void SetRemainingPieceCount(int32 NewRemainingPieceCount);
	void SetEliminated(bool bNewIsEliminated);

	UFUNCTION()
	void OnRep_PlayerIndex();

	UFUNCTION()
	void OnRep_RemainingPieceCount();

	UFUNCTION()
	void OnRep_IsEliminated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Player")
	void OnPlayerIndexChanged(int32 NewPlayerIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Table")
	void OnRemainingPieceCountChanged(int32 NewRemainingPieceCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnEliminatedChanged(bool bNewIsEliminated);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
