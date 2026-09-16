#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPGameState.generated.h"

class APlayerState;
class ACameraActor;

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

	virtual void BeginPlay() override;

	// 레벨에 배치된 고정 카메라(Actor Tag로 식별: "TopViewCamera" / "DeathQuarterViewCamera")
	UFUNCTION(BlueprintPure, Category = "Camera")
	ACameraActor* GetTopViewCamera() const { return TopViewCamera; }

	UFUNCTION(BlueprintPure, Category = "Camera")
	ACameraActor* GetDeathQuarterViewCamera() const { return DeathQuarterViewCamera; }

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

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	float MatchStartServerWorldTime = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Match")
	float MatchEndServerWorldTime = 0.0f;

	FOnReplicatedTurnStateChanged OnReplicatedTurnStateChanged;

	void SetMatchPhase(ETabulletMatchPhase NewPhase);
	void SetCurrentTurnPlayerState(APlayerState* NewTurnPlayerState, int32 NewTurnNumber);
	void SetTurnPhase(ETabulletTurnPhase NewTurnPhase);
	void SetTurnOrderPlayerStates(const TArray<TObjectPtr<APlayerState>>& NewTurnOrderPlayerStates);
	void SetWinnerPlayerState(APlayerState* NewWinnerPlayerState);

	UFUNCTION(BlueprintPure, Category = "Match")
	float GetMatchElapsedSeconds() const;

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

protected:
	UPROPERTY()
	TObjectPtr<ACameraActor> TopViewCamera;

	UPROPERTY()
	TObjectPtr<ACameraActor> DeathQuarterViewCamera;
};
