#include "TPPlayerController.h"

#include "TPGameMode.h"
#include "TPGameState.h"
#include "Component/ViewModeComponent.h"
#include "EngineUtils.h"
#include "TPPlayerState.h"
#include "Table/Components/TableFlickInputComponent.h"
#include "Table/Actors/FlickTableBase.h"

ATPPlayerController::ATPPlayerController()
{
	TableFlickInputComponent = CreateDefaultSubobject<UTableFlickInputComponent>(TEXT("TableFlickInputComponent"));
}

void ATPPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BindGameStateInputEvents();
	RefreshMouseInputMode();
}

void ATPPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RefreshMouseInputMode();
}

void ATPPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	RefreshMouseInputMode();
}

void ATPPlayerController::ServerRequestFlick_Implementation(AFlickTableBase* Table, ATableBulletPiece* Piece, FVector WorldDirection, float NormalizedPower)
{
	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->RequestFlick(this, Table, Piece, WorldDirection, NormalizedPower);
	}
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

	if (bShowTopDownCursor)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetIgnoreLookInput(true);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetIgnoreLookInput(false);
	}
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
	const ATPGameState* TPGameState = GetWorld() ? GetWorld()->GetGameState<ATPGameState>() : nullptr;
	return TPGameState && TPGameState->MatchPhase == ETabulletMatchPhase::InGame;
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

	return IsTopDownViewMode();
}
