// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModeComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

// Sets default values for this component's properties
UViewModeComponent::UViewModeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(false);
}

// Called when the game starts
void UViewModeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
	
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
}

void UViewModeComponent::SetViewMode(EViewMode NewMode)
{
	if (CurrentMode == NewMode || !SpringArm)
	{
		return;
	}
	
	CurrentMode = NewMode;
	
	// 탑뷰에서 마우스로 시점 안 돌아감
	SpringArm->bUsePawnControlRotation = (CurrentMode == EViewMode::FirstPerson);
	
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->bUseControllerRotationYaw = (CurrentMode == EViewMode::FirstPerson);
	}
	
	SetComponentTickEnabled(true);
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

// Called every frame
void UViewModeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SpringArm)
	{
		SetComponentTickEnabled(false);
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
	
	if (CurrentMode == EViewMode::TopDown)
	{
		FRotator CurrentRot = SpringArm->GetComponentRotation();
		CurrentRot.Pitch = FMath::FInterpTo(CurrentRot.Pitch, TargetPitch, DeltaTime, BlendSpeed);
		CurrentRot.Yaw = FMath::FInterpTo(CurrentRot.Yaw, TopDownYaw, DeltaTime, BlendSpeed);
		CurrentRot.Roll = 0.f;
		SpringArm->SetWorldRotation(CurrentRot);
	}
	
	const bool bArmDone = FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetArmLength, 1.f);
	const bool bOffsetDone = SpringArm->GetRelativeLocation().Equals(TargetOffset, 1.f);
	
	if (bArmDone && bOffsetDone)
	{
		SpringArm->TargetArmLength = TargetArmLength;
		SpringArm->SetRelativeLocation(TargetOffset);
		SetComponentTickEnabled(false);
	}
}

