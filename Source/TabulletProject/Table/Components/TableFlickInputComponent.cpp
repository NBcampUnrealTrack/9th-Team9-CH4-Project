// Fill out your copyright notice in the Description page of Project Settings.


#include "TableFlickInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Engine/EngineTypes.h"
#include "TabulletProject/Table/Actors/FlickTableBase.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"
#include "Camera/PlayerCameraManager.h"
#include "Math/RotationMatrix.h"
#include "GameFramework/PlayerState.h"


// Sets default values for this component's properties
UTableFlickInputComponent::UTableFlickInputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UTableFlickInputComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetOwner());

	if (!IsValid(PlayerController))
	{
		UE_LOG(LogTemp,	Error, TEXT("TableFlickInputComponent must be attached ""to a PlayerController."));

		return;
	}

	if (!PlayerController->IsLocalController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer =	PlayerController->GetLocalPlayer();

	if (!IsValid(LocalPlayer))
	{
		return;
	}
	
	InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (!IsValid(InputSubsystem))
	{
		return;
	}
	
	UEnhancedInputComponent* EnhancedInputComponent =Cast<UEnhancedInputComponent>(PlayerController->InputComponent);

	if (!IsValid(EnhancedInputComponent)
		|| !IsValid(FlickAction))
	{
		return;
	}

	// 좌클 누르기
	EnhancedInputComponent->BindAction(FlickAction, ETriggerEvent::Started, this,	&UTableFlickInputComponent::HandleFlickStarted);

	//좌클 떼기
	EnhancedInputComponent->BindAction(FlickAction, ETriggerEvent::Completed, this, &UTableFlickInputComponent::HandleFlickCompleted);
}

void UTableFlickInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(InputSubsystem) && IsValid(TableMappingContext))
	{
		InputSubsystem->RemoveMappingContext(TableMappingContext);
	}

	if (IsValid(PlayerController))
	{
		PlayerController->SetIgnoreLookInput(false);
	}

	SetComponentTickEnabled(false);
	SelectedPiece = nullptr;
	ActiveTable = nullptr;
	bTableInputEnabled = false;
	bDragging = false;

	Super::EndPlay(EndPlayReason);
}

void UTableFlickInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDragging || !IsValid(SelectedPiece) || !IsValid(ActiveTable) || !IsValid(PlayerController))
	{
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;

	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	const FVector2D DragVector = FVector2D(MouseX, MouseY) - DragStartScreenPosition;
	const float DragDistance = DragVector.Size();

	if (DragDistance < MinDragDistancePixels)
	{
		return;
	}

	const float NormalizedPower = FMath::Clamp(DragDistance / MaxDragDistancePixels, 0.0f, 1.0f);
	const FVector2D FlickScreenDirection = -DragVector.GetSafeNormal();
	const FRotationMatrix CameraMatrix(PlayerController->PlayerCameraManager->GetCameraRotation());
	const FVector CameraRight = CameraMatrix.GetUnitAxis(EAxis::Y);
	const FVector CameraUp = CameraMatrix.GetUnitAxis(EAxis::Z);
	const FVector WorldDirection = CameraRight * FlickScreenDirection.X - CameraUp * FlickScreenDirection.Y;
	const FVector TableDirection = FVector::VectorPlaneProject(WorldDirection, ActiveTable->GetActorUpVector()).GetSafeNormal();

	const float PreviewLength = MaxPreviewLength * NormalizedPower;
	const FVector ArrowStart = SelectedPiece->GetActorLocation() + ActiveTable->GetActorUpVector() * 10.0f;
	const FVector ArrowEnd = ArrowStart + TableDirection * PreviewLength;

	DrawDebugDirectionalArrow(GetWorld(), ArrowStart, ArrowEnd, PreviewArrowSize, FColor::Green, false, 0.0f, 0, PreviewLineThickness);
}

void UTableFlickInputComponent::HandleFlickStarted(const FInputActionValue& InputValue)
{
	if (!bTableInputEnabled || !IsValid(ActiveTable))
	{
		return;
	}

	SelectedPiece = FindPieceUnderCursor();

	if (!IsValid(SelectedPiece))
	{
		UE_LOG(LogTemp,	Log, TEXT("No table piece under cursor"));

		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;

	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		SelectedPiece = nullptr;
		return;
	}
	

	DragStartScreenPosition = FVector2D(MouseX, MouseY);

	bDragging = true;
	SetComponentTickEnabled(true);
	PlayerController->SetIgnoreLookInput(true);

	UE_LOG(LogTemp, Log, TEXT("Selected table piece: %s"), *SelectedPiece->GetName());
}

void UTableFlickInputComponent::HandleFlickCompleted(const FInputActionValue& InputValue)
{
	if (!bTableInputEnabled || !IsValid(ActiveTable) || !bDragging || !IsValid(SelectedPiece))
	{
		if (IsValid(PlayerController))
		{
			PlayerController->SetIgnoreLookInput(false);
		}

		SetComponentTickEnabled(false);
		bDragging = false;
		SelectedPiece = nullptr;
		return;
	}
	
	float MouseX = 0.0f;
	float MouseY = 0.0f;
	
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		PlayerController->SetIgnoreLookInput(false);
		SetComponentTickEnabled(false);
		bDragging = false;
		SelectedPiece = nullptr;
		return;
	}
	
	const FVector2D CurrentMousePosition(MouseX, MouseY);

	const FVector2D DragVector = CurrentMousePosition - DragStartScreenPosition;

	const float DragDistance = DragVector.Size();
	
	if (DragDistance < MinDragDistancePixels)
	{
		PlayerController->SetIgnoreLookInput(false);
		SetComponentTickEnabled(false);
		bDragging = false;
		SelectedPiece = nullptr;
		return;
	}

	const float NormalizedPower =FMath::Clamp(DragDistance / MaxDragDistancePixels, 0.0f, 1.0f);

	const FVector2D FlickScreenDirection = -DragVector.GetSafeNormal();

	UE_LOG(LogTemp, Log, TEXT("Flick Direction X=%.2f Y=%.2f, ""Power=%.2f"),
		FlickScreenDirection.X,
		FlickScreenDirection.Y,
		NormalizedPower);
	
	if (!IsValid(PlayerController->PlayerCameraManager))
	{
		PlayerController->SetIgnoreLookInput(false);
		SetComponentTickEnabled(false);
		bDragging = false;
		SelectedPiece = nullptr;
		return;
	}

	const FRotator CameraRotation =	PlayerController->PlayerCameraManager->GetCameraRotation();

	const FRotationMatrix CameraMatrix(CameraRotation);

	const FVector CameraRight =	CameraMatrix.GetUnitAxis(EAxis::Y);

	const FVector CameraUp = CameraMatrix.GetUnitAxis(EAxis::Z);

	const FVector WorldDirection = CameraRight * FlickScreenDirection.X	- CameraUp * FlickScreenDirection.Y;

	ServerRequestFlick(SelectedPiece, ActiveTable, WorldDirection, NormalizedPower);
	
	bTableInputEnabled = false;
	InputSubsystem->RemoveMappingContext(TableMappingContext);

	PlayerController->SetIgnoreLookInput(false);
	SetComponentTickEnabled(false);
	bDragging = false;
	SelectedPiece = nullptr;
}

ATableBulletPiece* UTableFlickInputComponent::FindPieceUnderCursor() const
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}

	FHitResult HitResult;

	const bool bHit = PlayerController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

	if (!bHit)
	{
		return nullptr;
	}

	ATableBulletPiece* HitPiece = Cast<ATableBulletPiece>(HitResult.GetActor());

	if (!IsValid(HitPiece) || HitPiece->IsOut())
	{
		return nullptr;
	}
	
	APlayerState* LocalPlayerState = PlayerController->GetPlayerState<APlayerState>();

	if (!HitPiece->IsOwnedByPlayerState(LocalPlayerState))
	{
		return nullptr;
	}

	return HitPiece;
}

void UTableFlickInputComponent::SetTableInputEnabled(bool bEnabled, AFlickTableBase* InTable)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController() || !IsValid(InputSubsystem) || !IsValid(TableMappingContext))
	{
		return;
	}

	if (bEnabled)
	{
		if (!IsValid(InTable))
		{
			return;
		}

		ActiveTable = InTable;
		bTableInputEnabled = true;

		InputSubsystem->AddMappingContext(TableMappingContext, MappingPriority);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(TableMappingContext);
		PlayerController->SetIgnoreLookInput(false);
		SetComponentTickEnabled(false);

		SelectedPiece = nullptr;
		ActiveTable = nullptr;
		bTableInputEnabled = false;
	}
}

void UTableFlickInputComponent::ServerRequestFlick_Implementation(ATableBulletPiece* Piece, AFlickTableBase* Table,
	FVector WorldDirection, float NormalizedPower)
{
	if (!IsValid(Piece) || !IsValid(Table) || Piece->IsOut() || !IsValid(PlayerController))
	{
		return;
	}

	APlayerState* RequestingPlayerState = PlayerController->GetPlayerState<APlayerState>();

	if (!Piece->IsOwnedByPlayerState(RequestingPlayerState))
	{
		UE_LOG(LogTemp, Warning, TEXT("Rejected flick request: player does not own piece %s"), *Piece->GetName());
		return;
	}

	Table->TryApplyFlick(Piece, WorldDirection, NormalizedPower);
}
