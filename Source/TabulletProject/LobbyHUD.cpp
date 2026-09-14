// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

ALobbyHUD::ALobbyHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> LobbyWidgetFinder(TEXT("/Game/Tabullet/UI/WBP_Lobby"));
	if (LobbyWidgetFinder.Succeeded())
	{
		LobbyWidgetClass = LobbyWidgetFinder.Class;
	}
}

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !PlayerController->IsLocalController() || !LobbyWidgetClass)
	{
		return;
	}

	LobbyWidget = CreateWidget<UUserWidget>(PlayerController, LobbyWidgetClass);
	if (!LobbyWidget)
	{
		return;
	}

	LobbyWidget->AddToViewport();

	PlayerController->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
}
