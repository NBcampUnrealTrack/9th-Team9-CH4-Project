// Fill out your copyright notice in the Description page of Project Settings.

#include "AimAndShootComponent.h"
#include "TabulletProject/Character/TPCharacter.h"
#include "InteractionComponent.h"
#include "WeaponManagerComponent.h"

UAimAndShootComponent::UAimAndShootComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAimAndShootComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 이 컴포가 붙어있는 캐릭 찾기
	OwnerCharacter = GetOwner<ATPCharacter>();
	if (!OwnerCharacter)
		return;
	
	// 캐릭터가 가지고 있는 상호작용 컴포넌트 찾기
	Interaction = OwnerCharacter->FindComponentByClass<UInteractionComponent>();
	
	// 캐릭터가 가지고 있는 무기 관리 컴포 찾기
	WeaponManager = OwnerCharacter->FindComponentByClass<UWeaponManagerComponent>();
}

bool UAimAndShootComponent::CanStartFire() const
{
	if (!Interaction || !WeaponManager)
		return false;
	
	return Interaction->CanShoot();
}

void UAimAndShootComponent::HandleFireStarted(const FInputActionValue& Value)
{
	if (!CanStartFire())
	{
		return;
	}
	
	UE_LOG(LogTemp,Warning, TEXT("[AimAndShoot] Shootable - current weapon shoot"));
	
	WeaponManager->FireCurrentWeapon();
}

