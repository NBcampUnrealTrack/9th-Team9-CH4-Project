// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DrawDebugHelpers.h"
#include "TableFlickInputComponent.generated.h"

class APlayerController;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ATableBulletPiece;
class AFlickTableBase;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS(ClassGroup=(Table), meta=(BlueprintSpawnableComponent))
class TABULLETPROJECT_API UTableFlickInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTableFlickInputComponent();

	UFUNCTION(BlueprintCallable, Category = "Table | Input")
	void SetTableInputEnabled(bool bEnabled, AFlickTableBase* InTable);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Table | Input")
	TObjectPtr<UInputMappingContext> TableMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table|Input")
	TObjectPtr<UInputAction> FlickAction;
	
	UPROPERTY(EditDefaultsOnly,	BlueprintReadOnly, Category = "Table|Input")
	int32 MappingPriority = 10;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;
	
	void HandleFlickStarted(const FInputActionValue& InputValue);

	void HandleFlickCompleted(const FInputActionValue& InputValue);
	
	UPROPERTY(Transient)
	TObjectPtr<ATableBulletPiece> SelectedPiece;
	
	ATableBulletPiece* FindPieceUnderCursor() const;
	
	UPROPERTY(Transient)
	TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;

	UPROPERTY(Transient)
	TObjectPtr<AFlickTableBase> ActiveTable;

	bool bTableInputEnabled = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table | Input", meta = (ClampMin = "1.0"))
	float MaxDragDistancePixels = 400.f;
	
	FVector2D DragStartScreenPosition = FVector2D::ZeroVector;
	
	bool bDragging = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table|Input", meta = (ClampMin = "0.0"))
	float MinDragDistancePixels = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table|Input|Preview", meta = (ClampMin = "0.0"))
	float MaxPreviewLength = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table|Input|Preview", meta = (ClampMin = "0.0"))
	float PreviewArrowSize = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Table|Input|Preview", meta = (ClampMin = "0.0"))
	float PreviewLineThickness = 1.0f;
	
	UFUNCTION(Server, Reliable)
	void ServerRequestFlick(ATableBulletPiece* Piece, AFlickTableBase* Table, FVector WorldDirection, float NormalizedPower);
};
