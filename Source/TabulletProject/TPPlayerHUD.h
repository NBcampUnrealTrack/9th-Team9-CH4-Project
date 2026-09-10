// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WeaponType.h"
#include "TPPlayerHUD.generated.h"

class UUserWidget;
class UTextBlock;
class UProgressBar;

/**
 * 
 */
UCLASS()
class TABULLETPROJECT_API ATPPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATPPlayerHUD();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameHUDWidgetClass;

private:
	void CacheHUDWidgets();
	void RefreshHUD();
	void UpdateMatchStatusState(float DeltaSeconds);
	FText GetPlayerCountText() const;
	FText GetMatchStatusText() const;
	FText GetTurnText() const;
	FText GetAmmoText(EWeaponType WeaponType, const TCHAR* Label) const;
	void RefreshHealthHUD();

	UPROPERTY()
	TObjectPtr<UUserWidget> GameHUDWidget;

	UPROPERTY()
	TObjectPtr<UTextBlock> PlayerCountText;

	UPROPERTY()
	TObjectPtr<UTextBlock> MatchStatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> TurnText;

	UPROPERTY()
	TObjectPtr<UTextBlock> RevolverAmmoText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ShotgunAmmoText;

	UPROPERTY()
	TObjectPtr<UTextBlock> SniperAmmoText;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> HealthText;

	float RefreshElapsedTime = 0.0f;
	float GameStartMessageElapsedTime = 0.0f;
	float ShootingPhaseMessageElapsedTime = 0.0f;
	int32 LastObservedMatchPhase = INDEX_NONE;
	bool bShowGameStartMessage = false;
	bool bShowShootingPhaseMessage = false;
};
