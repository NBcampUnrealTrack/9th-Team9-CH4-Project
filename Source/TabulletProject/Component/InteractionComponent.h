// Interaction.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class ATPGameState;
class ATPPlayerState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();
	
	// 캐릭터의 턴?
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsMyTurn() const;
	
	// 알까기 할 수 있나
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanFlick() const;
	
	// 사격 할 수 있나
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool CanShoot() const;

protected:
	const ATPGameState* GetTPGameState() const;
	const ATPPlayerState* GetOwnerPlayerState() const;
};
