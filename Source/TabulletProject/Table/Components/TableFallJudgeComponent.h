// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "TableFallJudgeComponent.generated.h"

class ATableBulletPiece;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTablePieceEnteredFallJudge, ATableBulletPiece*, FallenPiece);

UCLASS(ClassGroup=(Table), meta=(BlueprintSpawnableComponent))
class TABULLETPROJECT_API UTableFallJudgeComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTableFallJudgeComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "Table | Fall Judge")
	FOnTablePieceEnteredFallJudge OnPieceEnteredFallJudge;

	// 빠른 물체가 Overlap 이벤트를 놓쳤을 때 사용할 보조 낙하 판정이다.
	bool HasPassedBelowDetectionPlane(const FVector& WorldLocation) const;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
