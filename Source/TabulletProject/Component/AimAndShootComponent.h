// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AimAndShootComponent.generated.h"

class ATPCharacter;
class UInteractionComponent;
class UWeaponManagerComponent;
struct FInputActionValue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UAimAndShootComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAimAndShootComponent();
	
	// 발사 버튼 누르면 호출
	void HandleFireStarted(const FInputActionValue& Value);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// 총을 쏠 수 있는 상태인지 확인
	bool CanStartFire() const;
	
	UPROPERTY(Transient)
	TObjectPtr<ATPCharacter> OwnerCharacter;
	
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> Interaction;
	
	UPROPERTY(Transient)
	TObjectPtr<UWeaponManagerComponent> WeaponManager;
};
