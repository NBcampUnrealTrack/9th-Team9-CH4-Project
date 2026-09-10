#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/OnlineReplStructs.h"
#include "TimerManager.h"
#include "TPGameMode.generated.h"

class APlayerState;
class AFlickTableBase;
class ATableBulletPiece;
class ATPCharacter;
enum class EWeaponType : uint8;

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
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	bool CanStartGame() const;
	void StartGame();
	bool RequestFlick(AController* RequestingController, AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);
	void NotifyShotResolved(AController* ShootingController);

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
	void InitializeTurnOrder(bool bResetCombatEliminations);
	void SpawnTablePieces();
	AFlickTableBase* FindFlickTable() const;
	void StartNextTableRound();
	void StartFirstTurn();
	void SetCurrentTurnByIndex(int32 NewTurnIndex);
	void CheckResolveComplete();
	bool IsPlayerStartOccupied(const AActor* PlayerStart) const;
	AActor* FindPlayerStartByTag(FName StartTag, bool bRequireUnoccupied) const;
	bool UpdateEliminationsAndCheckGameOver();
	void StartShootingPhase(APlayerState* TableWinner);
	void BuildShootingTurnOrder();
	void AdvanceShootingTurn();
	bool IsCurrentShootingTurnController(AController* Controller) const;
	bool CheckShootingGameOver();
	bool IsPlayerAlive(APlayerState* PlayerState) const;
	bool HasAnyAmmo(APlayerState* PlayerState) const;
	int32 GetTotalAmmoCount(APlayerState* PlayerState) const;
	ATPCharacter* GetCharacterForPlayerState(APlayerState* PlayerState) const;
	void AwardAmmoForFallenPiece(ATableBulletPiece* FallenPiece, APlayerState* PieceOwner);
	void FinishGame(APlayerState* Winner);

	UFUNCTION()
	void HandleTablePieceFell(ATableBulletPiece* FallenPiece, APlayerState* PieceOwner);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "1"))
	int32 RequiredPlayerCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Match", meta = (ClampMin = "0.0"))
	float AutoStartDelay = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn", meta = (ClampMin = "0"))
	int32 PiecesPerPlayer = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Spawn", meta = (ClampMin = "0"))
	int32 SpecialPieceCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn", meta = (ClampMin = "0.01"))
	float ResolveCheckInterval = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turn", meta = (ClampMin = "0.1"))
	float MaxResolveSeconds = 8.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug | Character")
	TArray<TSubclassOf<ATPCharacter>> DebugCharacterClasses;

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> TurnOrder;

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> ShootingTurnOrder;

	UPROPERTY()
	TArray<TObjectPtr<APlayerState>> TableEliminationOrder;

	UPROPERTY()
	TObjectPtr<APlayerState> TablePhaseWinner;

	UPROPERTY()
	TObjectPtr<APlayerState> LastFlickPlayerState;

	int32 CurrentTurnIndex = INDEX_NONE;
	int32 CurrentShootingTurnIndex = INDEX_NONE;
	FTimerHandle StartMatchTimerHandle;
	FTimerHandle ResolveCheckTimerHandle;
	float ResolveStartedTime = 0.0f;
	bool bTablePiecesSpawned = false;
};
