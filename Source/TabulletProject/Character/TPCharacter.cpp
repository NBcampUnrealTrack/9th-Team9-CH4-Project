// Fill out your copyright notice in the Description page of Project Settings.

#include "TPCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "../Component/HeadMovementComponent.h"
#include "../Component/ViewModeComponent.h"
#include "../Component/InteractionComponent.h"
#include "../Component/AimAndShootComponent.h"
#include "../Component/AmmoComponent.h"
#include "../Component/HealthComponent.h"

// Sets default values
ATPCharacter::ATPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh(), TEXT("CameraSocket"));
	SpringArm->SetUsingAbsoluteScale(true);
	SpringArm->TargetArmLength = 0.f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	HeadMovement = CreateDefaultSubobject<UHeadMovementComponent>(TEXT("HeadMovement"));
	Interaction = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	AimAndShoot = CreateDefaultSubobject<UAimAndShootComponent>(TEXT("AimAndShoot"));
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
}

// Called when the game starts or when spawned
void ATPCharacter::BeginPlay()
{
	Super::BeginPlay();
	ViewModeComp = FindComponentByClass<UViewModeComponent>();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	
	// 발사 테스트용
	if (HasAuthority())
	{
		if (UAmmoComponent* Ammo = FindComponentByClass<UAmmoComponent>())
		{
			Ammo->SetAmmoCount(EWeaponType::Revolver, 10);
			Ammo->SetAmmoCount(EWeaponType::Shotgun, 10);
			Ammo->SetAmmoCount(EWeaponType::Sniper, 10);
		}
	}
}

void ATPCharacter::DebugDamage(float Amount)
{
	ServerDebugDamage(Amount);
}

void ATPCharacter::ServerDebugDamage_Implementation(float Amount)
{
	if (Health)
	{
		const float Applied = Health->ApplyHealthDamage(Amount, GetController());
		UE_LOG(LogTemp, Warning, TEXT("DebugDamage: %.1f applied, Health now %.1f"),
			Applied, Health->GetHealth());
	}
}

// Called to bind functionality to input
void ATPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPCharacter::Look);
		}
		
		if (StretchLeftAction)
		{
			EnhancedInput->BindAction(StretchLeftAction, ETriggerEvent::Started, this, &ATPCharacter::StretchLeft);
		}
		
		if (StretchCenterAction)
		{
			EnhancedInput->BindAction(StretchCenterAction, ETriggerEvent::Started, this, &ATPCharacter::StretchCenter);
		}
		
		if (StretchRightAction)
		{
			EnhancedInput->BindAction(StretchRightAction, ETriggerEvent::Started, this, &ATPCharacter::StretchRight);
		}
		
		if (HeadTiltAction)
		{
			EnhancedInput->BindAction(HeadTiltAction, ETriggerEvent::Triggered, this, &ATPCharacter::HeadTilt);
			EnhancedInput->BindAction(HeadTiltAction, ETriggerEvent::Completed, this , &ATPCharacter::HeadTiltReleased);
			EnhancedInput->BindAction(HeadTiltAction, ETriggerEvent::Canceled, this, &ATPCharacter::HeadTiltReleased);
		}
		
		if (FireAction && AimAndShoot)
		{
			EnhancedInput->BindAction(FireAction.Get(), ETriggerEvent::Started, AimAndShoot.Get(), &UAimAndShootComponent::HandleFireStarted);
		}
	}
}

void ATPCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	if (ViewModeComp && !ViewModeComp->IsFirstPerson())
	{
		return; 
	}
	
	if (!Controller)
	{
		return;
	}
	
	if (HeadMovement && HeadMovement->GetIsStretched())
	{
		HeadMovement->AddHeadRotation(LookAxisVector.X, LookAxisVector.Y);
		return;
	}
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ATPCharacter::StretchLeft()
{
	if (ViewModeComp && !ViewModeComp->IsFirstPerson())
	{
		return;
	}
	
	if (HeadMovement)
	{
		HeadMovement->ToggleStretch(ENeckStretchDirection::Left);
	}
}

void ATPCharacter::StretchCenter()
{
	if (ViewModeComp && !ViewModeComp->IsFirstPerson())
	{
		return;
	}
	
	if (HeadMovement)
	{
		HeadMovement->ToggleStretch(ENeckStretchDirection::Center);
	}
}

void ATPCharacter::StretchRight()
{
	if (ViewModeComp && !ViewModeComp->IsFirstPerson())
	{
		return;
	}
	
	if (HeadMovement)
	{
		HeadMovement->ToggleStretch(ENeckStretchDirection::Right);
	}
}

void ATPCharacter::HeadTiltReleased()
{
	if (HeadMovement)
	{
		HeadMovement->SetTiltInput(0.f);
	}
}

void ATPCharacter::HeadTilt(const FInputActionValue& Value)
{
	if (ViewModeComp && !ViewModeComp->IsFirstPerson())
	{
		return;
	}
	
	if (HeadMovement)
	{
		HeadMovement->SetTiltInput(Value.Get<float>());
	}
}

