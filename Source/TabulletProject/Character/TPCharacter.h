// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHeadMovementComponent;
class UViewModeComponent;
struct FInputActionValue;

UCLASS()
class TABULLETPROJECT_API ATPCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATPCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Look(const FInputActionValue& Value);
	void HeadTilt(const FInputActionValue& Value);
	void HeadTiltReleased();
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HeadMovement")
	TObjectPtr<UHeadMovementComponent> HeadMovement;
	
	UPROPERTY()
	TObjectPtr<UViewModeComponent> ViewModeComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> StretchLeftAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> StretchCenterAction;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> StretchRightAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> HeadTiltAction;
    
    void StretchLeft();
    void StretchCenter();
    void StretchRight();
};
