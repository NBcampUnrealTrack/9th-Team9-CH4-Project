// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModeComponent.h"
#include "HeadMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "../TPGameState.h"
#include "../TPPlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UViewModeComponent::UViewModeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetIsReplicatedByDefault(false);
}

// Called when the game starts
void UViewModeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
	HeadMovement = GetOwner()->FindComponentByClass<UHeadMovementComponent>();
	
	if (!SpringArm)
	{
		UE_LOG(LogTemp, Error, TEXT("ViewModeComponent: SpringArm을 찾을 수 없음"));
		return;
	}
	
	// 스폰된 방향이 탑뷰 기준 -> 스폰 방향은 테이블을 바라보게 해야됨
	TopDownYaw = GetOwner()->GetActorRotation().Yaw;
	
	float ArmLength;
	FVector Offset;
	float Pitch;
	GetTargetValues(ArmLength, Offset, Pitch);
	
	SpringArm->TargetArmLength = ArmLength;
	SpringArm->SetRelativeLocation(Offset);
	UpdateRotationSource();

	TryBindGameState();
}

void UViewModeComponent::TryBindGameState()
{
	if (bBoundToGameState)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	GameStateRef = GetWorld()->GetGameState<ATPGameState>();
	if (!GameStateRef)
	{
		return;
	}

	GameStateRef->OnReplicatedTurnStateChanged.AddUObject(
		this, &UViewModeComponent::UpdateViewModeFromPhase);
	bBoundToGameState = true;
	UpdateViewModeFromPhase();
}

void UViewModeComponent::UpdateViewModeFromPhase()
{
	if (!GameStateRef)
	{
		return;
	}

	switch (GameStateRef->MatchPhase)
	{
	case ETabulletMatchPhase::InGame:
		SetViewMode(EViewMode::TopDown);
		break;
		
	case ETabulletMatchPhase::ShootingPhase:
		SetViewMode(EViewMode::FirstPerson);
		break;
		
	default:
		break;
	}
}

void UViewModeComponent::SetViewMode(EViewMode NewMode)
{
	if (CurrentMode == NewMode || !SpringArm)
	{
		return;
	}
	
	CurrentMode = NewMode;
	Blending = true;

	if (CurrentMode == EViewMode::TopDown && HeadMovement)
	{
		HeadMovement->ResetStretch();
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	if (CurrentMode == EViewMode::FirstPerson && OwnerController)
	{
		// 탑뷰 동안 어긋난 몸 방향과 시선을 다시 맞춤
		FRotator SyncRot = OwnerPawn->GetActorRotation();
		SyncRot.Pitch = 0.f;
		SyncRot.Roll = 0.f;
		OwnerController->SetControlRotation(SyncRot);
	}
	
	if (ATPPlayerController* TPPC = Cast<ATPPlayerController>(OwnerController))
	{
		TPPC->RefreshMouseInputMode();
	}
}

void UViewModeComponent::GetTargetValues(float& OutArmLength, FVector& OutOffset, float& OutPitch) const
{
	if (CurrentMode == EViewMode::FirstPerson)
	{
		OutArmLength = FirstPersonArmLength;
		OutOffset = FirstPersonOffset;
		OutPitch = 0.f;
	}
	else
	{
		OutArmLength = TopDownArmLength;
		OutOffset = TopDownOffset;
		OutPitch = TopDownPitch;
	}
}

void UViewModeComponent::UpdateRotationSource()
{
	if (!SpringArm)
	{
		return;
	}
	
	// 탑뷰 : 마우스로 시점 안 돌아감
	// 1인칭 : 목이 늘어난 동안은 머리 회전 따라감
	const bool Stretched = HeadMovement && HeadMovement->GetIsStretched();
	SpringArm->bUsePawnControlRotation = IsFirstPerson() && !Stretched;
	
	// 컨트롤러 회전을 안 쓰는 동안에는 소켓 기울기 보정
	if (IsFirstPerson() && Stretched)
	{
		SpringArm->SetRelativeRotation(FirstPersonRotationOffset);
	}
	
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->bUseControllerRotationYaw = IsFirstPerson();
	}
}

// Called every frame
void UViewModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	if (!SpringArm)
	{
		return;
	}

	TryBindGameState();
	UpdateRotationSource();

	if (CurrentMode == EViewMode::TopDown)
	{
		FRotator CurrentRot = SpringArm->GetComponentRotation();
		CurrentRot.Pitch = FMath::FInterpTo(CurrentRot.Pitch, TopDownPitch, DeltaTime, BlendSpeed);
		CurrentRot.Yaw = FMath::FInterpTo(CurrentRot.Yaw, TopDownYaw, DeltaTime, BlendSpeed);
		CurrentRot.Roll = 0.f;
		SpringArm->SetWorldRotation(CurrentRot);
	}
	
	if (!Blending)
	{
		return;
	}
	
	float TargetArmLength;
	FVector TargetOffset;
	float TargetPitch;
	GetTargetValues(TargetArmLength, TargetOffset, TargetPitch);
	
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength, TargetArmLength, DeltaTime, BlendSpeed);
	
	SpringArm->SetRelativeLocation(FMath::VInterpTo(
		SpringArm->GetRelativeLocation(), TargetOffset, DeltaTime, BlendSpeed));
	
	const bool ArmDone = FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetArmLength, 1.f);
	const bool OffsetDone = SpringArm->GetRelativeLocation().Equals(TargetOffset, 1.f);
	
	if (ArmDone && OffsetDone)
	{
		SpringArm->TargetArmLength = TargetArmLength;
		SpringArm->SetRelativeLocation(TargetOffset);
		Blending = false;
	}
}

