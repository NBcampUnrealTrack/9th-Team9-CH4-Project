16// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerState.h"

#include "Net/UnrealNetwork.h"

void ATPPlayerState::SetReady(bool bReady)
{
	if (HasAuthority())
	{
		bIsReady = bReady;
	}
}

void ATPPlayerState::OnRep_IsReady()
{
}

void ATPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerState, bIsReady);
}

