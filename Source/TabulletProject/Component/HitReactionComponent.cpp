// Fill out your copyright notice in the Description page of Project Settings.

#include "HitReactionComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
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
	ApplyRagdoll();
	OnDeathVisualApplied.Broadcast();
}

void UHitReactionComponent::ApplyRagdoll()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	// 카메라가 메쉬(래그돌 뼈)를 따라가지 않도록 그 순간 시점을 분리해서 고정
	if (USpringArmComponent* CameraBoom = OwnerCharacter->FindComponentByClass<USpringArmComponent>())
	{
		CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		// bUsePawnControlRotation이 켜져 있으면 분리 후에도 마우스 회전을 계속 따라가므로 같이 꺼줌
		CameraBoom->bUsePawnControlRotation = false;
		CameraBoom->SetComponentTickEnabled(false);
	}

	// 캡슐/무브먼트가 더 이상 물리 시뮬레이션에 간섭하지 않도록 정지
	if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
	{
		MovementComp->DisableMovement();
		MovementComp->SetComponentTickEnabled(false);
	}

	if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 메쉬를 물리 시뮬레이션으로 전환해 축 처지는 래그돌 연출
	if (USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh())
	{
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetCollisionObjectType(ECC_PhysicsBody);
		Mesh->SetSimulatePhysics(true);
		Mesh->SetAllBodiesSimulatePhysics(true);
		Mesh->WakeAllRigidBodies();
		Mesh->bBlendPhysics = true;
	}
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