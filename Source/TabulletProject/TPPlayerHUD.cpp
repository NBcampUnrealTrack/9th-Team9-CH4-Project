// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "TPGameState.h"
#include "TPPlayerController.h"
#include "TPPlayerState.h"

ATPPlayerHUD::ATPPlayerHUD()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATPPlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController() || !GameHUDWidgetClass)
	{
		return;
	}

	GameHUDWidget = CreateWidget<UUserWidget>(PlayerController, GameHUDWidgetClass);
	if (GameHUDWidget)
	{
		GameHUDWidget->AddToViewport();
		CacheHUDWidgets();
		RefreshHUD();
	}
}

void ATPPlayerHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMatchStatusState(DeltaSeconds);

	RefreshElapsedTime += DeltaSeconds;
	if (RefreshElapsedTime < 0.1f)
	{
		return;
	}

	RefreshElapsedTime = 0.0f;
	RefreshHUD();
}

void ATPPlayerHUD::CacheHUDWidgets()
{
	if (!GameHUDWidget)
	{
		return;
	}

	PlayerCountText = Cast<UTextBlock>(GameHUDWidget->GetWidgetFromName(TEXT("PlayerCountText")));
	MatchStatusText = Cast<UTextBlock>(GameHUDWidget->GetWidgetFromName(TEXT("MatchStatusText")));
	TurnText = Cast<UTextBlock>(GameHUDWidget->GetWidgetFromName(TEXT("TurnText")));

	UE_LOG(LogTemp, Log, TEXT("TPPlayerHUD widget cache. PlayerCountText=%s MatchStatusText=%s TurnText=%s"),
		PlayerCountText ? TEXT("Found") : TEXT("Missing"),
		MatchStatusText ? TEXT("Found") : TEXT("Missing"),
		TurnText ? TEXT("Found") : TEXT("Missing"));
}

void ATPPlayerHUD::RefreshHUD()
{
	if (PlayerCountText)
	{
		PlayerCountText->SetText(GetPlayerCountText());
	}

	if (MatchStatusText)
	{
		MatchStatusText->SetText(GetMatchStatusText());
	}

	if (TurnText)
	{
		TurnText->SetText(GetTurnText());
	}
}

void ATPPlayerHUD::UpdateMatchStatusState(float DeltaSeconds)
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState)
	{
		return;
	}

	const int32 CurrentMatchPhase = static_cast<int32>(TPGameState->MatchPhase);
	if (LastObservedMatchPhase != CurrentMatchPhase)
	{
		LastObservedMatchPhase = CurrentMatchPhase;
		if (TPGameState->MatchPhase == ETabulletMatchPhase::InGame)
		{
			bShowGameStartMessage = true;
			GameStartMessageElapsedTime = 0.0f;
		}
	}

	if (bShowGameStartMessage)
	{
		GameStartMessageElapsedTime += DeltaSeconds;
		if (GameStartMessageElapsedTime >= 1.0f)
		{
			bShowGameStartMessage = false;
		}
	}
}

FText ATPPlayerHUD::GetPlayerCountText() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	const int32 ConnectedPlayerCount = TPGameState ? TPGameState->PlayerArray.Num() : 0;

	return FText::Format(NSLOCTEXT("TPPlayerHUD", "PlayerCountFormat", "Players {0} / 4"), ConnectedPlayerCount);
}

FText ATPPlayerHUD::GetMatchStatusText() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState)
	{
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusUnknown", "Waiting");
	}

	switch (TPGameState->MatchPhase)
	{
	case ETabulletMatchPhase::WaitingForPlayers:
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusWaitingForPlayers", "Waiting for players");
	case ETabulletMatchPhase::Starting:
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusStarting", "Starting");
	case ETabulletMatchPhase::InGame:
		return bShowGameStartMessage
			? NSLOCTEXT("TPPlayerHUD", "MatchStatusGameStart", "Game Start")
			: FText::GetEmpty();
	case ETabulletMatchPhase::ShootingPhase:
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusShootingPhase", "Shooting Phase");
	case ETabulletMatchPhase::GameOver:
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusGameOver", "Game Over");
	default:
		return NSLOCTEXT("TPPlayerHUD", "MatchStatusFallback", "Waiting");
	}
}

FText ATPPlayerHUD::GetTurnText() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState || (TPGameState->MatchPhase != ETabulletMatchPhase::InGame && TPGameState->MatchPhase != ETabulletMatchPhase::ShootingPhase))
	{
		return FText::GetEmpty();
	}

	const ATPPlayerController* TPPlayerController = Cast<ATPPlayerController>(GetOwningPlayerController());
	if (TPPlayerController && TPPlayerController->IsMyTurn())
	{
		if (TPGameState->MatchPhase == ETabulletMatchPhase::ShootingPhase)
		{
			return FText::Format(NSLOCTEXT("TPPlayerHUD", "YourShotFormat", "Shot {0} - Your Turn"), TPGameState->TurnNumber);
		}

		return FText::Format(NSLOCTEXT("TPPlayerHUD", "YourTurnFormat", "Turn {0} - Your Turn"), TPGameState->TurnNumber);
	}

	const ATPPlayerState* CurrentTurnPlayerState = Cast<ATPPlayerState>(TPGameState->CurrentTurnPlayerState);
	if (!CurrentTurnPlayerState)
	{
		return FText::Format(NSLOCTEXT("TPPlayerHUD", "TurnWaitingFormat", "Turn {0}"), TPGameState->TurnNumber);
	}

	return FText::Format(
		TPGameState->MatchPhase == ETabulletMatchPhase::ShootingPhase
			? NSLOCTEXT("TPPlayerHUD", "OtherPlayerShotFormat", "Shot {0} - Player {1} Turn")
			: NSLOCTEXT("TPPlayerHUD", "OtherPlayerTurnFormat", "Turn {0} - Player {1} Turn"),
		TPGameState->TurnNumber,
		CurrentTurnPlayerState->PlayerIndex + 1);
}
