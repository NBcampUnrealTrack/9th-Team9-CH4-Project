#include "TablePhysicsResolutionComponent.h"

#include "TabulletProject/Table/Actors/TableBulletPiece.h"
#include "TabulletProject/Table/Components/TableFallJudgeComponent.h"
#include "TabulletProject/Table/Core/TableLog.h"

UTablePhysicsResolutionComponent::UTablePhysicsResolutionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTablePhysicsResolutionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UTablePhysicsResolutionComponent::StartMonitoring(const TArray<TObjectPtr<ATableBulletPiece>>& Pieces, UTableFallJudgeComponent* InFallJudge, float InLinearSpeedThreshold, float InAngularSpeedThreshold, float InRequiredSettledTime)
{
	MonitoredPieces = Pieces;
	FallJudge = InFallJudge;
	LinearSpeedThreshold = FMath::Max(0.0f, InLinearSpeedThreshold);
	AngularSpeedThreshold = FMath::Max(0.0f, InAngularSpeedThreshold);
	RequiredSettledTime = FMath::Max(0.0f, InRequiredSettledTime);
	SettledElapsedTime = 0.0f;
	bMonitoringMovement = true;
	SetComponentTickEnabled(true);
}

void UTablePhysicsResolutionComponent::ResetMonitoring()
{
	MonitoredPieces.Reset();
	FallJudge = nullptr;
	SettledElapsedTime = 0.0f;
	bMonitoringMovement = false;
	SetComponentTickEnabled(false);
}

void UTablePhysicsResolutionComponent::ForceFinish()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (ATableBulletPiece* Piece : MonitoredPieces)
	{
		if (IsValid(Piece))
		{
			Piece->StopPhysicsMovement();
		}
	}

	UE_LOG(LogTable, Warning, TEXT("Flick resolution timed out. Forcing table resolution to finish."));
	ResetMonitoring();
}

void UTablePhysicsResolutionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority() || !bMonitoringMovement)
	{
		return;
	}

	MonitoredPieces.RemoveAll([](const TObjectPtr<ATableBulletPiece>& Piece)
	{
		return !IsValid(Piece);
	});

	TArray<TObjectPtr<ATableBulletPiece>> MissedFallenPieces;
	if (IsValid(FallJudge))
	{
		for (ATableBulletPiece* Piece : MonitoredPieces)
		{
			if (IsValid(Piece) && FallJudge->HasPassedBelowDetectionPlane(Piece->GetActorLocation()))
			{
				MissedFallenPieces.Add(Piece);
			}
		}
	}

	for (ATableBulletPiece* Piece : MissedFallenPieces)
	{
		UE_LOG(LogTable, Warning, TEXT("Recovered missed fall overlap: %s"), *Piece->GetName());
		OnMissedFall.Broadcast(Piece);
	}

	bool bAnyPieceMoving = false;
	for (ATableBulletPiece* Piece : MonitoredPieces)
	{
		if (IsValid(Piece) && Piece->IsMoving(LinearSpeedThreshold, AngularSpeedThreshold))
		{
			bAnyPieceMoving = true;
			break;
		}
	}

	if (bAnyPieceMoving)
	{
		SettledElapsedTime = 0.0f;
		return;
	}

	SettledElapsedTime += DeltaTime;
	if (SettledElapsedTime < RequiredSettledTime)
	{
		return;
	}

	ResetMonitoring();
	OnPhysicsSettled.Broadcast();
}
