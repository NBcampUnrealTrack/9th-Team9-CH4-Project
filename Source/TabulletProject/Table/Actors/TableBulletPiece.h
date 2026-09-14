// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TabulletProject/Table/Core/TableTypes.h"
#include "TabulletProject/WeaponType.h"
#include "TableBulletPiece.generated.h"

class APlayerState;
class UStaticMeshComponent;
enum class EWeaponType : uint8;

UCLASS()
class TABULLETPROJECT_API ATableBulletPiece : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableBulletPiece();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void MarkAsOut();
	
	UFUNCTION(BlueprintPure, Category = "Table Piece")
	bool IsOut() const	{ return PieceState == ETablePieceState::Out; }

	bool ApplyFlickImpulse(const FVector& WorldImpulse);

	bool IsMoving(float LinearThreshold, float AngularThreshold) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Movement")
	void StopPhysicsMovement();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Ownership")
	void SetOwningPlayerState(APlayerState* InOwningPlayerState);

	UFUNCTION(BlueprintPure, Category = "Table | Ownership")
	APlayerState* GetOwningPlayerState() const;

	UFUNCTION(BlueprintPure, Category = "Table | Ownership")
	bool IsOwnedByPlayerState(const APlayerState* PlayerState) const;
	
	UFUNCTION(BlueprintPure, Category = "Table Piece")
	ETablePieceType GetPieceType() const { return PieceType; }
	
	UFUNCTION(BlueprintPure, Category = "Table Piece")
	EWeaponType GetRewardWeaponType() const { return RewardWeaponType; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<UStaticMeshComponent> PieceMesh;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category = "Table")
	ETablePieceType PieceType = ETablePieceType::Normal;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Piece", meta = (EditCondition = "PieceType == ETablePieceType::Special", EditConditionHides))
	EWeaponType RewardWeaponType;
	
	UPROPERTY(ReplicatedUsing = OnRep_PieceState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Table Piece")
	ETablePieceState PieceState = ETablePieceState::OnTable;
	
	UPROPERTY(ReplicatedUsing = OnRep_OwningPlayerState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Table Piece")
	TObjectPtr<APlayerState> OwningPlayerState;

	UFUNCTION()
	void OnRep_OwningPlayerState();

	void UpdateOwnershipHighlight();
	
	UFUNCTION()
	void OnRep_PieceState();

	void ApplyOutState();
};
