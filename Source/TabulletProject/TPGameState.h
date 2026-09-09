#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPGameState.generated.h"

class APlayerState;

DECLARE_MULTICAST_DELEGATE(FOnReplicatedTurnStateChanged);

UENUM(BlueprintType)
enum class ETabulletMatchPhase : uint8
{
	WaitingForPlayers,
	Starting,
	InGame,
	ShootingPhase,
	GameOver
};

UENUM(BlueprintType)
enum class ETabulletTurnPhase : uint8
{
	None,
	WaitingForAction,
	ResolvingPhysics,
	WaitingForShot
};

UCLASS()
class TABULLETPROJECT_API ATPGameState : public AGameState
{
	GENERATED_BODY()

public:
	ATPGameState();

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Match")
	ETabulletMatchPhase MatchPhase = ETabulletMatchPhase::WaitingForPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnPlayerState, BlueprintReadOnly, Category = "Turn")
	TObjectPtr<APlayerState> CurrentTurnPlayerState;

	UPROPERTY(ReplicatedUsing = OnRep_TurnNumber, BlueprintReadOnly, Category = "Turn")
	int32 TurnNumber = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TurnPhase, BlueprintReadOnly, Category = "Turn")
	ETabulletTurnPhase TurnPhase = ETabulletTurnPhase::None;

	UPROPERTY(ReplicatedUsing = OnRep_TurnOrderPlayerStates, BlueprintReadOnly, Category = "Turn")
	TArray<TObjectPtr<APlayerState>> TurnOrderPlayerStates;

	UPROPERTY(ReplicatedUsing = OnRep_WinnerPlayerState, BlueprintReadOnly, Category = "Match")
	TObjectPtr<APlayerState> WinnerPlayerState;

	FOnReplicatedTurnStateChanged OnReplicatedTurnStateChanged;

	void SetMatchPhase(ETabulletMatchPhase NewPhase);
	void SetCurrentTurnPlayerState(APlayerState* NewTurnPlayerState, int32 NewTurnNumber);
	void SetTurnPhase(ETabulletTurnPhase NewTurnPhase);
	void SetTurnOrderPlayerStates(const TArray<TObjectPtr<APlayerState>>& NewTurnOrderPlayerStates);
	void SetWinnerPlayerState(APlayerState* NewWinnerPlayerState);

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_CurrentTurnPlayerState();

	UFUNCTION()
	void OnRep_TurnNumber();

	UFUNCTION()
	void OnRep_TurnPhase();

	UFUNCTION()
	void OnRep_TurnOrderPlayerStates();

	UFUNCTION()
	void OnRep_WinnerPlayerState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnMatchPhaseChanged(ETabulletMatchPhase NewPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Turn")
	void OnTurnChanged(APlayerState* NewTurnPlayerState, int32 NewTurnNumber);

	UFUNCTION(BlueprintImplementableEvent, Category = "Turn")
	void OnTurnPhaseChanged(ETabulletTurnPhase NewTurnPhase);

	UFUNCTION(BlueprintImplementableEvent, Category = "Turn")
	void OnTurnOrderChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Match")
	void OnWinnerChanged(APlayerState* NewWinnerPlayerState);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
