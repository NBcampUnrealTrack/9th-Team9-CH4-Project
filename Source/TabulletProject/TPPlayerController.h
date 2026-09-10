#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPPlayerController.generated.h"

class AFlickTableBase;
class ATableBulletPiece;
class UTableFlickInputComponent;
class ATPGameState;
class UUserWidget;
/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATPPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Table | Flick")
	void ServerRequestFlick(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);

	UFUNCTION(BlueprintPure, Category = "Turn")
	bool IsMyTurn() const;

	UFUNCTION(BlueprintPure, Category = "Table | Input")
	UTableFlickInputComponent* GetTableFlickInputComponent() const { return TableFlickInputComponent; }

	UFUNCTION(BlueprintCallable, Category = "Input")
	void RefreshMouseInputMode();
	
	void ShowCrosshair();
	void HideCrosshair();

protected:
	void BindGameStateInputEvents();
	AFlickTableBase* FindFlickTable();
	bool IsTopDownViewMode() const;
	bool ShouldEnableTableInput() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Table | Input")
	TObjectPtr<UTableFlickInputComponent> TableFlickInputComponent;

	UPROPERTY()
	TObjectPtr<ATPGameState> BoundGameState;

	UPROPERTY()
	TObjectPtr<AFlickTableBase> CachedFlickTable;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> CrosshairWidgetInstance;
};
