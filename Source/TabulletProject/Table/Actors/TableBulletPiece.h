// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TabulletProject/Table/Core/TableTypes.h"
#include "TableBulletPiece.generated.h"

class APlayerState;
class UStaticMeshComponent;

UCLASS()
class TABULLETPROJECT_API ATableBulletPiece : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATableBulletPiece();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void MarkAsOut();
	
	UFUNCTION(BlueprintPure, Category = "Table Piece")
	bool IsOut() const	{ return PieceState == ETablePieceState::Out; }

	UFUNCTION(BlueprintPure, Category = "Table Piece")
	bool IsOwnedBy(const APlayerState* PlayerState) const;

	UFUNCTION(BlueprintPure, Category = "Table Piece")
	APlayerState* GetOwningPlayerState() const { return OwningPlayerState; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table Piece")
	void SetOwningPlayerState(APlayerState* NewOwningPlayerState);
	
	bool ApplyFlickImpulse(const FVector& WorldImpulse);

	bool IsMoving(float LinearThreshold, float AngularThreshold) const;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<UStaticMeshComponent> PieceMesh;
	
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category = "Table")
	ETablePieceType PieceType = ETablePieceType::Normal;
	
	UPROPERTY(ReplicatedUsing = OnRep_PieceState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Table Piece")
	ETablePieceState PieceState = ETablePieceState::OnTable;
	
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Table Piece")
	TObjectPtr<APlayerState> OwningPlayerState;
	
	UFUNCTION()
	void OnRep_PieceState();

	void ApplyOutState();
};
