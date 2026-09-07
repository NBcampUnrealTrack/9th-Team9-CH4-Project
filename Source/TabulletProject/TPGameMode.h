#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/OnlineReplStructs.h"
#include "TimerManager.h"
#include "TPGameMode.generated.h"

class APlayerState;
class AFlickTableBase;
class ATableBulletPiece;

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATPGameMode();

	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleMatchHasStarted() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	bool CanStartGame() const;
	void StartGame();
	bool RequestFlick(AController* RequestingController, AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Turn")
	void AdvanceTurn();

	UFUNCTION(BlueprintPure, Category = "Turn")
	APlayerState* GetCurrentTurnPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "Turn")
	bool IsCurrentTurnController(AController* Controller) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table Piece")
	void AssignPieceOwner(ATableBulletPiece* Piece, APlayerState* NewOwner);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Match")
	void RecalculatePlayerPieceCounts();

protected:
	void BeginStartCountdown();
	void CancelStartCountdown();
	void InitializeTurnOrder();
	void SpawnTablePieces();
	AFlickTableBase* FindFlickTable() const;
	void StartFirstTurn();
	void SetCurrentTurnByIndex(int32 NewTurnIndex);
	void CheckResolveComplete();
	bool IsPlayerStartOccupied(const AActor* PlayerStart) const;
	AActor* FindPlayerStartByTag(FName StartTag, bool bRequireUnoccupied) const;
	bool AreAnyPiecesMoving() const;
	bool UpdateEliminationsAndCheckGameOver();
	void FinishGame(APlayerState* Winner);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "1"))
	int32 RequiredPlayerCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "0.0"))
	float AutoStartDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn", meta = (ClampMin = "0"))
	int32 PiecesPerPlayer = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn", meta = (ClampMin = "0"))
	int32 SpecialPieceCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn", meta = (ClampMin = "0.01"))
	float ResolveCheckInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn", meta = (ClampMin = "0.0"))
	float PieceStoppedSpeedThreshold = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn", meta = (ClampMin = "0.1"))
	float MaxResolveSeconds = 8.0f;

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> TurnOrder;

	int32 CurrentTurnIndex = INDEX_NONE;
	FTimerHandle StartMatchTimerHandle;
	FTimerHandle ResolveCheckTimerHandle;
	float ResolveStartedTime = 0.0f;
	bool bTablePiecesSpawned = false;
};
