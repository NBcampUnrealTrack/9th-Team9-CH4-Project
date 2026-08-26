#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ATPGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void HandleMatchHasStarted() override;

	void SetPlayerReady(AController* Player, bool bReady);
	bool CanStartGame() const;
	void StartGame();
};
