#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPPlayerController.generated.h"

class AFlickTableBase;
class ATableBulletPiece;

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Ready")
	void ServerSetReady(bool bReady);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Table | Flick")
	void ServerRequestFlick(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower);

	UFUNCTION(BlueprintPure, Category = "Turn")
	bool IsMyTurn() const;
};
