// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
}

float UHealthComponent::ApplyHealthDamage(float DamageAmount, AController* Killer)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return 0.f;
	}
	
	if (DamageAmount <= 0.f || IsDead())
	{
		return 0.f;
	}

	const float OldHealth = Health;
	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);
	const float ActualDamage = OldHealth - Health;

	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (Health <= 0.f)
	{
		OnDeath.Broadcast(Killer);
		OnDeathVisual.Broadcast();
	}

	return ActualDamage;
}

void UHealthComponent::Heal(float HealAmount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (HealAmount <= 0.f || IsDead())
	{
		return;
	}
	
	Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UHealthComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
	
	if (Health <= 0.f)
	{
		OnDeathVisual.Broadcast();
	}
}

