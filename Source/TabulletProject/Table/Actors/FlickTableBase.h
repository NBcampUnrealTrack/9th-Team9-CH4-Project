// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TabulletProject/Table/Core/TableTypes.h"
#include "TabulletProject/WeaponType.h"
#include "FlickTableBase.generated.h"

class ATableBulletPiece;
class UTableFallJudgeComponent;
class APlayerState;
class UTablePieceSpawnComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTablePiecesSettled);
UCLASS()
class TABULLETPROJECT_API AFlickTableBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlickTableBase();
	
	UPROPERTY(BlueprintAssignable, Category = "Table | Movement")
	FOnTablePiecesSettled OnTablePiecesSettled;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpecialPieceCaptured, APlayerState*, CapturingPlayer, EWeaponType, WeaponType);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Flick")
	bool TryApplyFlick(ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Piece Registry")
	bool RegisterPiece(ATableBulletPiece* Piece);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Piece Registry")
	bool UnregisterPiece(ATableBulletPiece* Piece);

	UFUNCTION(BlueprintPure, Category = "Table | Piece Registry")
	int32 GetRegisteredPieceCount() const;
	
	UFUNCTION(BlueprintPure, Category = "Table | Piece Registry")
	int32 GetRemainingPieceCountForPlayer(const APlayerState* PlayerState) const;
	
	UFUNCTION(BlueprintPure, Category = "Table | Piece Registry")
	int32 GetRegisteredPieceCountByType(ETablePieceType PieceType) const;
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Spawn")
	int32 SpawnNormalPiecesForPlayers(const TArray<APlayerState*>& Players, int32 PiecesPerPlayer);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Spawn")
	int32 SpawnSpecialPieces(int32 PieceCount);
	
	UPROPERTY(BlueprintAssignable, Category = "Table | Reward")
	FOnSpecialPieceCaptured OnSpecialPieceCaptured;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<UTableFallJudgeComponent> FallJudge;
	
	UFUNCTION()
	void HandlePieceEnteredFallJudge(ATableBulletPiece* FallenPiece);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Flick", meta = (ClampMin = "0.0"))
	float MaxFlickImpulse = 40.0f;		// 최대 파워인데 나중에 수정해야 함. 현재는 테스트하면서 해본 임시값
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Movement", meta = (ClampMin = "0.0"))
	float LinearSpeedThreshold = 2.0f;			// 무시 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Movement", meta = (ClampMin = "0.0"))
	float AngularSpeedThreshold = 2.0f;			// 무시 회전 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Movement", meta = (ClampMin = "0.0"))
	float RequiredSettledTime = 0.5f;			// 무시까지 걸리는 시간

	float SettledElapsedTime = 0.0f;

	bool bMonitoringPieceMovement = false;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<ATableBulletPiece>> RegisteredPieces;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<UTablePieceSpawnComponent> PieceSpawner;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Spawn")
	TObjectPtr<USceneComponent> Player1SpawnOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Spawn")
	TObjectPtr<USceneComponent> Player2SpawnOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Spawn")
	TObjectPtr<USceneComponent> Player3SpawnOrigin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Spawn")
	TObjectPtr<USceneComponent> Player4SpawnOrigin;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Spawn")
	TObjectPtr<USceneComponent> SpecialPieceSpawnOrigin;
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerState> ActiveFlickPlayerState;
};
