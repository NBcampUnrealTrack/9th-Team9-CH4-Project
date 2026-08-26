#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPGameState.generated.h"

UENUM(BlueprintType)
enum class ETabulletMatchPhase : uint8
{
	WaitingForPlayers,
	ReadyCheck,
	InGame
};

UCLASS()
class TABULLETPROJECT_API ATPGameState : public AGameState
{
	GENERATED_BODY()

public:
	ATPGameState();

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	ETabulletMatchPhase MatchPhase = ETabulletMatchPhase::WaitingForPlayers;

	void SetMatchPhase(ETabulletMatchPhase NewPhase);

	UFUNCTION()
	void OnRep_MatchPhase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
