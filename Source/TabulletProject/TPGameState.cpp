// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameState.h"

#include "Net/UnrealNetwork.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"

ATPGameState::ATPGameState()
{
	MatchPhase = ETabulletMatchPhase::WaitingForPlayers;
}

void ATPGameState::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에 배치된 고정 카메라를 Actor Tag로 찾아 캐싱 (모든 클라이언트가 동일한 레벨 액터를 각자 로컬에서 찾음)
	for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
	{
		ACameraActor* CameraActor = *It;
		if (CameraActor->ActorHasTag(TEXT("TopViewCamera")))
		{
			TopViewCamera = CameraActor;
		}
		else if (CameraActor->ActorHasTag(TEXT("DeathQuarterViewCamera")))
		{
			DeathQuarterViewCamera = CameraActor;
		}
	}
}

void ATPGameState::SetMatchPhase(ETabulletMatchPhase NewPhase)
{
	if (HasAuthority() && MatchPhase != NewPhase)
	{
		MatchPhase = NewPhase;
		if (MatchPhase == ETabulletMatchPhase::InGame && MatchStartServerWorldTime <= 0.0f)
		{
			MatchStartServerWorldTime = GetServerWorldTimeSeconds();
			MatchEndServerWorldTime = 0.0f;
		}
		else if (MatchPhase == ETabulletMatchPhase::GameOver && MatchEndServerWorldTime <= 0.0f)
		{
			MatchEndServerWorldTime = GetServerWorldTimeSeconds();
		}
		else if (MatchPhase == ETabulletMatchPhase::WaitingForPlayers)
		{
			MatchStartServerWorldTime = 0.0f;
			MatchEndServerWorldTime = 0.0f;
		}

		OnMatchPhaseChanged(MatchPhase);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

float ATPGameState::GetMatchElapsedSeconds() const
{
	if (MatchStartServerWorldTime <= 0.0f)
	{
		return 0.0f;
	}

	const float CurrentServerWorldTime = MatchEndServerWorldTime > 0.0f
		? MatchEndServerWorldTime
		: GetServerWorldTimeSeconds();

	return FMath::Max(0.0f, CurrentServerWorldTime - MatchStartServerWorldTime);
}

void ATPGameState::SetCurrentTurnPlayerState(APlayerState* NewTurnPlayerState, int32 NewTurnNumber)
{
	if (HasAuthority() && (CurrentTurnPlayerState != NewTurnPlayerState || TurnNumber != NewTurnNumber))
	{
		CurrentTurnPlayerState = NewTurnPlayerState;
		TurnNumber = NewTurnNumber;
		OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

void ATPGameState::SetTurnPhase(ETabulletTurnPhase NewTurnPhase)
{
	if (HasAuthority() && TurnPhase != NewTurnPhase)
	{
		TurnPhase = NewTurnPhase;
		OnTurnPhaseChanged(TurnPhase);
		OnReplicatedTurnStateChanged.Broadcast();
	}
}

void ATPGameState::SetTurnOrderPlayerStates(const TArray<TObjectPtr<APlayerState>>& NewTurnOrderPlayerStates)
{
	if (HasAuthority())
	{
		TurnOrderPlayerStates = NewTurnOrderPlayerStates;
		OnTurnOrderChanged();
	}
}

void ATPGameState::SetWinnerPlayerState(APlayerState* NewWinnerPlayerState)
{
	if (HasAuthority() && WinnerPlayerState != NewWinnerPlayerState)
	{
		WinnerPlayerState = NewWinnerPlayerState;
		OnWinnerChanged(WinnerPlayerState);
	}
}

void ATPGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged(MatchPhase);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_CurrentTurnPlayerState()
{
	OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnNumber()
{
	OnTurnChanged(CurrentTurnPlayerState, TurnNumber);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnPhase()
{
	OnTurnPhaseChanged(TurnPhase);
	OnReplicatedTurnStateChanged.Broadcast();
}

void ATPGameState::OnRep_TurnOrderPlayerStates()
{
	OnTurnOrderChanged();
}

void ATPGameState::OnRep_WinnerPlayerState()
{
	OnWinnerChanged(WinnerPlayerState);
}

void ATPGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPGameState, MatchPhase);
	DOREPLIFETIME(ATPGameState, CurrentTurnPlayerState);
	DOREPLIFETIME(ATPGameState, TurnNumber);
	DOREPLIFETIME(ATPGameState, TurnPhase);
	DOREPLIFETIME(ATPGameState, TurnOrderPlayerStates);
	DOREPLIFETIME(ATPGameState, WinnerPlayerState);
	DOREPLIFETIME(ATPGameState, MatchStartServerWorldTime);
	DOREPLIFETIME(ATPGameState, MatchEndServerWorldTime);
}

