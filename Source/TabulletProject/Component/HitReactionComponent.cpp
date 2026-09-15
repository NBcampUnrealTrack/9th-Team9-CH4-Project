// Fill out your copyright notice in the Description page of Project Settings.

#include "HitReactionComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
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

	// 죽은 뒤엔 마우스/키 입력이 Look, HeadTilt, Stretch, Fire 등 Pawn 쪽 입력에 계속 반응하지 않도록 차단.
	// T/F 시점 전환은 PlayerController의 InputComponent에 바인딩되어 있어 영향받지 않는다.
	if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
	{
		OwnerCharacter->DisableInput(PC);
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
		// AnimBP(목 늘리기 등 Transform Bone 노드)가 계속 본을 강제로 움직이면
		// 물리 시뮬레이션과 충돌해 랙돌이 튕겨나간다. 애니메이션 갱신 자체를 끊어서 방지.
		Mesh->SetAnimInstanceClass(nullptr);

		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetCollisionObjectType(ECC_PhysicsBody);
		// 기본 Ragdoll 프리셋은 Pawn 채널도 Block이라, 물리 시작 시 주변 캐릭터 캡슐과 겹쳐 있으면
		// depenetration으로 서로 튕겨나간다. 바닥/벽과는 부딪히되 다른 캐릭터와는 상호작용하지 않도록 덮어씀.
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
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