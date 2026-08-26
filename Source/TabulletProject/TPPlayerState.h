// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(ReplicatedUsing = OnRep_IsReady)
	bool bIsReady = false;
	
	void SetReady(bool bReady);

	UFUNCTION()
	void OnRep_IsReady();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
