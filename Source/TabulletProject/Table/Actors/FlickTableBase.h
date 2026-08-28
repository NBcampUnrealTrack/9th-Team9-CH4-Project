// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TabulletProject/Table/Core/TableTypes.h"
#include "FlickTableBase.generated.h"

class ATableBulletPiece;
class UTableFallJudgeComponent;

UCLASS()
class TABULLETPROJECT_API AFlickTableBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFlickTableBase();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Table | Flick")
	bool TryApplyFlick(ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table")
	TObjectPtr<UTableFallJudgeComponent> FallJudge;
	
	UFUNCTION()
	void HandlePieceEnteredFallJudge(ATableBulletPiece* FallenPiece);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Table | Flick", meta = (ClampMin = "0.0"))
	float MaxFlickImpulse = 40.0f;		// 최대 파워인데 나중에 수정해야 함. 현재는 테스트하면서 해본 임시값
};
