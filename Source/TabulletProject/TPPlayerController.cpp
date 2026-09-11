#include "TPPlayerController.h"

#include "TPGameMode.h"
#include "TPGameState.h"
#include "Component/ViewModeComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "TPPlayerState.h"
#include "TabulletProject/Component/WeaponManagerComponent.h"
#include "TabulletProject/Component/AmmoComponent.h"
#include "WeaponBase.h"
#include "Blueprint/UserWidget.h"
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
	
	if (bShowTopDownCursor)
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
	// 내 턴이어도 1인칭으로 전환돼 있으면 사격 페이즈처럼 마우스로 자유롭게 시점을 움직일 수 있어야 함
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
