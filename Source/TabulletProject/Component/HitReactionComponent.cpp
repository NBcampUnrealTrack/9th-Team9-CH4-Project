// Fill out your copyright notice in the Description page of Project Settings.

#include "HitReactionComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
// #include "NiagaraFunctionLibrary.h"

UHitReactionComponent::UHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsRunningDedicatedServer())
	{
		return;
	}
	
	if (UHealthComponent* HealthComp = GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		LastKnownHealth = HealthComp->GetHealth();
		
		HealthComp->OnHealthChanged.AddDynamic(this, &UHitReactionComponent::HandleHealthChanged);
		HealthComp->OnDeathVisual.AddDynamic(this, &UHitReactionComponent::HandleDeathVisual);
	}
}

void UHitReactionComponent::HandleHealthChanged(float NewHealth, float MaxHealth)
{
	if (NewHealth < LastKnownHealth && NewHealth > 0.f)
	{
		PlayHitMontage();
		SpawnHitEffect();
	}
	
	LastKnownHealth = NewHealth;
}

void UHitReactionComponent::HandleDeathVisual()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !DeathMontage)
	{
		OnDeathMontageFinished.Broadcast();
		return;
	}
	
	UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		OnDeathMontageFinished.Broadcast();
		return;
	}
	
	AnimInstance->Montage_Play(DeathMontage);
	
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UHitReactionComponent::OnDeathMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);
}

void UHitReactionComponent::OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnDeathMontageFinished.Broadcast();
}

void UHitReactionComponent::PlayHitMontage()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !HitMontage)
	{
		return;
	}
	
	if (UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(HitMontage);
	}
}

void UHitReactionComponent::SpawnHitEffect()
{
	if (!HitEffect)
	{
		return;
	}
	
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(), HitEffect, GetOwner()->GetActorLocation());
	
	//UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		//GetWorld(), HitEffect, GetOwner()->GetActorLocation());
}