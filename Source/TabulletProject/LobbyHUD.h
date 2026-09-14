// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

class UUserWidget;

UCLASS()
class TABULLETPROJECT_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALobbyHUD();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby | UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyWidget;
};
