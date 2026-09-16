#include "TPPlayerController.h"

#include "LobbyGameMode.h"
#include "TPGameMode.h"
#include "TPGameState.h"
#include "Component/ViewModeComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "TPPlayerState.h"
#include "TabulletProject/Component/WeaponManagerComponent.h"
#include "TabulletProject/Component/AmmoComponent.h"
#include "TabulletProject/Component/HealthComponent.h"
#include "WeaponBase.h"
#include "Blueprint/UserWidget.h"
#include "Table/Components/TableFlickInputComponent.h"
#include "Table/Actors/FlickTableBase.h"
#include "Engine/GameViewportClient.h"
#include "Net/UnrealNetwork.h"
#include "UnrealClient.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"

ATPPlayerController::ATPPlayerController()
{
	TableFlickInputComponent = CreateDefaultSubobject<UTableFlickInputComponent>(TEXT("TableFlickInputComponent"));
}

void ATPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BindGameStateInputEvents();
	RefreshMouseInputMode();

	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -PitchDownLimit;
		PlayerCameraManager->ViewPitchMax = PitchUpLimit;
	}
}

void ATPPlayerController::UpdateRotation(float DeltaTime)
{
	if (!bBaseYawInitialized)
	{
		if (const APawn* ControlledPawn = GetPawn())
		{
			BaseYaw = ControlledPawn->GetActorRotation().Yaw;
			bBaseYawInitialized = true;
		}
	}

	Super::UpdateRotation(DeltaTime);

	if (bBaseYawInitialized)
	{
		FRotator ClampedRotation = GetControlRotation();
		const float DeltaYaw = FMath::FindDeltaAngleDegrees(BaseYaw, ClampedRotation.Yaw);
		ClampedRotation.Yaw = BaseYaw + FMath::Clamp(DeltaYaw, -YawLimit, YawLimit);
		SetControlRotation(ClampedRotation);
	}
}

void ATPPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (ViewMappingContext)
		{
			Subsystem->AddMappingContext(ViewMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (TopViewAction)
		{
			EnhancedInput->BindAction(TopViewAction, ETriggerEvent::Started, this, &ATPPlayerController::HandleTopViewInput);
		}

		if (DeathQuarterViewAction)
		{
			EnhancedInput->BindAction(DeathQuarterViewAction, ETriggerEvent::Started, this, &ATPPlayerController::HandleDeathQuarterViewInput);
		}
	}
}

void ATPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RefreshMouseInputMode();

	if (UHealthComponent* HealthComp = InPawn ? InPawn->FindComponentByClass<UHealthComponent>() : nullptr)
	{
		HealthComp->OnDeathVisual.AddDynamic(this, &ATPPlayerController::HandleLocalPawnDeathVisual);
	}
}

void ATPPlayerController::OnUnPossess()
{
	if (APawn* CurrentPawn = GetPawn())
	{
		if (UHealthComponent* HealthComp = CurrentPawn->FindComponentByClass<UHealthComponent>())
		{
			HealthComp->OnDeathVisual.RemoveDynamic(this, &ATPPlayerController::HandleLocalPawnDeathVisual);
		}
	}

	Super::OnUnPossess();
}

void ATPPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	RefreshMouseInputMode();
}

void ATPPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	
	ApplyPendingInputMode();
}

void ATPPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerController, SelectedLobbyCharacterType);
}

void ATPPlayerController::ServerRequestFlick_Implementation(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	bool bAccepted = false;
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		bAccepted = TPGameMode->RequestFlick(this, Table, Piece, WorldDirection, NormalizedPower);
	}

	if (!bAccepted)
	{
		ClientFlickRequestRejected();
	}
}

void ATPPlayerController::ServerSetLobbyReady_Implementation(bool bReady)
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALobbyGameMode>() : nullptr)
	{
		LobbyGameMode->RequestSetPlayerReady(this, bReady);
	}
}

void ATPPlayerController::ServerSelectLobbyCharacter_Implementation(ETPCharacterType CharacterType)
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALobbyGameMode>() : nullptr)
	{
		LobbyGameMode->RequestSelectCharacter(this, CharacterType);
	}
}

void ATPPlayerController::ToggleLobbyReady()
{
	const ATPPlayerState* TPPlayerState = GetPlayerState<ATPPlayerState>();
	if (!TPPlayerState || (TPPlayerState->SelectedCharacterType == ETPCharacterType::None && SelectedLobbyCharacterType == ETPCharacterType::None))
	{
		return;
	}

	ServerSetLobbyReady(!TPPlayerState->bIsReady);
}

FText ATPPlayerController::GetLobbyReadyButtonText() const
{
	const ATPPlayerState* TPPlayerState = GetPlayerState<ATPPlayerState>();
	if (!TPPlayerState || TPPlayerState->SelectedCharacterType == ETPCharacterType::None)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyReadyButtonSelectCharacter", "Select Character");
	}

	return TPPlayerState->bIsReady
		? NSLOCTEXT("TPPlayerController", "LobbyReadyButtonCancelReady", "Cancel Ready")
		: NSLOCTEXT("TPPlayerController", "LobbyReadyButtonReady", "Ready");
}

FText ATPPlayerController::GetLobbyPlayerNameText(int32 SlotIndex) const
{
	const ATPPlayerState* LobbyPlayerState = GetLobbyPlayerStateAt(SlotIndex);
	if (!LobbyPlayerState)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyPlayerNameEmpty", "Empty");
	}

	const int32 DisplayIndex = LobbyPlayerState->PlayerIndex != INDEX_NONE
		? LobbyPlayerState->PlayerIndex + 1
		: SlotIndex + 1;

	return FText::Format(NSLOCTEXT("TPPlayerController", "LobbyPlayerNameFormat", "Player {0}"), DisplayIndex);
}

FText ATPPlayerController::GetLobbyPlayerCharacterText(int32 SlotIndex) const
{
	const ATPPlayerState* LobbyPlayerState = GetLobbyPlayerStateAt(SlotIndex);
	if (!LobbyPlayerState)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterEmpty", "-");
	}

	return GetCharacterTypeText(LobbyPlayerState->SelectedCharacterType);
}

FText ATPPlayerController::GetLobbyPlayerReadyText(int32 SlotIndex) const
{
	const ATPPlayerState* LobbyPlayerState = GetLobbyPlayerStateAt(SlotIndex);
	if (!LobbyPlayerState)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyReadyWaiting", "Waiting");
	}

	if (LobbyPlayerState->SelectedCharacterType == ETPCharacterType::None)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyReadySelecting", "Selecting");
	}

	return LobbyPlayerState->bIsReady
		? NSLOCTEXT("TPPlayerController", "LobbyReadyReady", "Ready")
		: NSLOCTEXT("TPPlayerController", "LobbyReadyNotReady", "Not Ready");
}

FText ATPPlayerController::GetLobbyPlayerCountText() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	const int32 ConnectedPlayerCount = TPGameState ? TPGameState->PlayerArray.Num() : 0;

	return FText::Format(NSLOCTEXT("TPPlayerController", "LobbyPlayerCountFormat", "Players {0} / 4"), ConnectedPlayerCount);
}

FText ATPPlayerController::GetLobbyStatusText() const
{
	const ATPPlayerState* TPPlayerState = GetPlayerState<ATPPlayerState>();
	if (!TPPlayerState || TPPlayerState->SelectedCharacterType == ETPCharacterType::None)
	{
		return NSLOCTEXT("TPPlayerController", "LobbyStatusSelectCharacter", "Select a character");
	}

	return TPPlayerState->bIsReady
		? NSLOCTEXT("TPPlayerController", "LobbyStatusWaitingPlayers", "Waiting for players")
		: NSLOCTEXT("TPPlayerController", "LobbyStatusPressReady", "Press Ready");
}

FText ATPPlayerController::GetCharacterTypeText(ETPCharacterType CharacterType)
{
	switch (CharacterType)
	{
	case ETPCharacterType::Dog:
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterDog", "Dog");
	case ETPCharacterType::Fox:
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterFox", "Fox");
	case ETPCharacterType::Bull:
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterBull", "Bull");
	case ETPCharacterType::Raccoon:
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterRaccoon", "Raccoon");
	case ETPCharacterType::None:
	default:
		return NSLOCTEXT("TPPlayerController", "LobbyCharacterNone", "None");
	}
}

const ATPPlayerState* ATPPlayerController::GetLobbyPlayerStateAt(int32 SlotIndex) const
{
	if (SlotIndex < 0)
	{
		return nullptr;
	}

	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState || !TPGameState->PlayerArray.IsValidIndex(SlotIndex))
	{
		return nullptr;
	}

	return Cast<ATPPlayerState>(TPGameState->PlayerArray[SlotIndex]);
}

void ATPPlayerController::SetSelectedLobbyCharacterType(ETPCharacterType NewCharacterType)
{
	if (HasAuthority())
	{
		SelectedLobbyCharacterType = NewCharacterType;
	}
}

void ATPPlayerController::ClientFlickRequestRejected_Implementation()
{
	if (!IsLocalController())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Flick request rejected by server. Restoring table input."));
	RefreshMouseInputMode();
}

bool ATPPlayerController::IsMyTurn() const
{
	if (const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr)
	{
		return PlayerState && TPGameState->CurrentTurnPlayerState == PlayerState;
	}

	return false;
}

void ATPPlayerController::RefreshMouseInputMode()
{
	if (!IsLocalController())
	{
		return;
	}

	BindGameStateInputEvents();

	const bool bShowTopDownCursor = IsTopDownViewMode();
	const bool bEnableTableInput = ShouldEnableTableInput();
	bShowMouseCursor = bShowTopDownCursor;

	if (TableFlickInputComponent)
	{
		AFlickTableBase* FlickTable = bEnableTableInput ? FindFlickTable() : nullptr;
		TableFlickInputComponent->SetTableInputEnabled(bEnableTableInput, FlickTable);
	}
	
	ResetIgnoreLookInput();
	SetIgnoreLookInput(bShowTopDownCursor);
	bShowMouseCursor = bShowTopDownCursor;
	
	if (bShowTopDownCursor)
	{
		HideCrosshair();
	}
	else
	{
		const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
		const bool bIsMyShootingTurn = TPGameState && PlayerState
			&& TPGameState->CurrentTurnPlayerState == PlayerState;

		bool bHasAmmo = false;
		if (const APawn* OwnedPawn = GetPawn())
		{
			const UWeaponManagerComponent* WeaponManager = OwnedPawn->FindComponentByClass<UWeaponManagerComponent>();
			const UAmmoComponent* AmmoComp = OwnedPawn->FindComponentByClass<UAmmoComponent>();

			if (WeaponManager && AmmoComp)
			{
				if (const AWeaponBase* CurrentWeapon = WeaponManager->GetCurrentWeapon())
				{
					bHasAmmo = AmmoComp->HasAmmo(CurrentWeapon->WeaponType);
				}
			}
		}

		if (bIsMyShootingTurn && bHasAmmo)
		{
			ShowCrosshair();
		}
		else
		{
			HideCrosshair();
		}
	}
	
	// 모드가 실제로 달라졌을 때만 적용 대상으로 표시한다
	if (!bHasAppliedInputMode || bLastAppliedTopDownCursor != bShowTopDownCursor)
	{
		bPendingTopDownCursor = bShowTopDownCursor;
		bInputModeDirty = true;
	}

	ApplyPendingInputMode();
}

bool ATPPlayerController::IsGameViewportFocused() const
{
	const UGameViewportClient* GameViewportClient = GetWorld() ? GetWorld()->GetGameViewport() : nullptr;
	const FViewport* GameViewport = GameViewportClient ? GameViewportClient->Viewport : nullptr;

	// 판단할 수 없으면 기존대로 적용한다
	return GameViewport == nullptr || GameViewport->HasFocus();
}

void ATPPlayerController::ApplyPendingInputMode()
{
	if (!bInputModeDirty)
	{
		return;
	}
	
	if (!IsGameViewportFocused())
	{
		return;
	}

	if (bPendingTopDownCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	bInputModeDirty = false;
	bHasAppliedInputMode = true;
	bLastAppliedTopDownCursor = bPendingTopDownCursor;
}

void ATPPlayerController::BindGameStateInputEvents()
{
	ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState || BoundGameState == TPGameState)
	{
		return;
	}

	if (BoundGameState)
	{
		BoundGameState->OnReplicatedTurnStateChanged.RemoveAll(this);
	}

	BoundGameState = TPGameState;
	BoundGameState->OnReplicatedTurnStateChanged.AddUObject(this, &ATPPlayerController::RefreshMouseInputMode);
}

AFlickTableBase* ATPPlayerController::FindFlickTable()
{
	if (IsValid(CachedFlickTable))
	{
		return CachedFlickTable;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AFlickTableBase> It(World); It; ++It)
	{
		if (AFlickTableBase* FlickTable = *It; IsValid(FlickTable))
		{
			CachedFlickTable = FlickTable;
			return CachedFlickTable;
		}
	}

	return nullptr;
}

bool ATPPlayerController::IsTopDownViewMode() const
{
	const APawn* MyPawn = GetPawn();
	const UViewModeComponent* PawnViewMode = MyPawn ? MyPawn->FindComponentByClass<UViewModeComponent>() : nullptr;
	return PawnViewMode && !PawnViewMode->IsFirstPerson();
}

bool ATPPlayerController::ShouldEnableTableInput() const
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	if (!TPGameState || !PlayerState)
	{
		return false;
	}

	if (TPGameState->MatchPhase != ETabulletMatchPhase::InGame
		|| TPGameState->TurnPhase != ETabulletTurnPhase::WaitingForAction
		|| TPGameState->CurrentTurnPlayerState != PlayerState)
	{
		return false;
	}

	const ATPPlayerState* TPPlayerState = GetPlayerState<ATPPlayerState>();
	if (TPPlayerState && TPPlayerState->bIsTableEliminated)
	{
		return false;
	}
	
	return true;
}

void ATPPlayerController::ShowCrosshair()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!CrosshairWidgetInstance && CrosshairWidgetClass)
	{
		CrosshairWidgetInstance = CreateWidget<UUserWidget>(this, CrosshairWidgetClass);
	}

	if (CrosshairWidgetInstance && !CrosshairWidgetInstance->IsInViewport())
	{
		CrosshairWidgetInstance->AddToViewport();
	}
}

void ATPPlayerController::HideCrosshair()
{
	if (CrosshairWidgetInstance && CrosshairWidgetInstance->IsInViewport())
	{
		CrosshairWidgetInstance->RemoveFromParent();
	}
}

void ATPPlayerController::HandleTopViewInput()
{
	APawn* MyPawn = GetPawn();
	const UHealthComponent* HealthComp = MyPawn ? MyPawn->FindComponentByClass<UHealthComponent>() : nullptr;

	if (HealthComp && HealthComp->IsDead())
	{
		const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
		SwitchToFixedCamera(TPGameState ? TPGameState->GetTopViewCamera() : nullptr);
		return;
	}

	if (UViewModeComponent* ViewModeComp = MyPawn ? MyPawn->FindComponentByClass<UViewModeComponent>() : nullptr)
	{
		ViewModeComp->SetViewMode(EViewMode::TopDown);
	}
}

void ATPPlayerController::HandleDeathQuarterViewInput()
{
	APawn* MyPawn = GetPawn();
	const UHealthComponent* HealthComp = MyPawn ? MyPawn->FindComponentByClass<UHealthComponent>() : nullptr;

	if (HealthComp && HealthComp->IsDead())
	{
		const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
		SwitchToFixedCamera(TPGameState ? TPGameState->GetDeathQuarterViewCamera() : nullptr);
		return;
	}

	if (UViewModeComponent* ViewModeComp = MyPawn ? MyPawn->FindComponentByClass<UViewModeComponent>() : nullptr)
	{
		ViewModeComp->SetViewMode(EViewMode::FirstPerson);
	}
}

void ATPPlayerController::HandleLocalPawnDeathVisual()
{
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	SwitchToFixedCamera(TPGameState ? TPGameState->GetTopViewCamera() : nullptr);
}

void ATPPlayerController::SwitchToFixedCamera(ACameraActor* TargetCamera)
{
	if (!IsLocalController() || !TargetCamera)
	{
		return;
	}

	SetViewTargetWithBlend(TargetCamera, DeathCameraBlendTime);
}
