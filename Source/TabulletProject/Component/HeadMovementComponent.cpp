// Fill out your copyright notice in the Description page of Project Settings.

#include "HeadMovementComponent.h"
#include "Net/UnrealNetwork.h"

UHeadMovementComponent::UHeadMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UHeadMovementComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHeadMovementComponent, CurrentStretch);
	DOREPLIFETIME(UHeadMovementComponent, IsStretched);
	DOREPLIFETIME(UHeadMovementComponent, StretchDirection);
	DOREPLIFETIME(UHeadMovementComponent, HeadRotationValue);
	DOREPLIFETIME(UHeadMovementComponent, CurrentTilt);
}

void UHeadMovementComponent::ToggleStretch(ENeckStretchDirection Direction)
{
	ServerToggleStretch(Direction);
}

void UHeadMovementComponent::ServerToggleStretch_Implementation(ENeckStretchDirection Direction)
{
	if (IsStretched && StretchDirection == Direction)
	{
		// 같은 방향에서 다시 누르면 목 원복
		IsStretched = false;
		HeadRotationValue = FVector::ZeroVector; // 머리 회전 초기화
	}
	else
	{
		// 새로 늘리거나 방향 전환
		IsStretched = true;
		StretchDirection = Direction;
	}
}

void UHeadMovementComponent::ResetStretch()
{
	LastSentTiltInput = 0.f;
	ServerResetStretch();
}

void UHeadMovementComponent::ServerResetStretch_Implementation()
{
	IsStretched = false;
	HeadRotationValue = FVector::ZeroVector;
	TiltInput = 0.f;
}

void UHeadMovementComponent::AddHeadRotation(float DeltaYaw, float DeltaPitch)
{
	ServerAddHeadRotation(DeltaYaw, DeltaPitch);
}

void UHeadMovementComponent::SetTiltInput(float Input)
{
	// 값이 바뀔 때만 서버에 전송
	if (FMath::IsNearlyEqual(Input, LastSentTiltInput))
	{
		return;
	}
	
	LastSentTiltInput = Input;
	ServerSetTiltInput(Input);
}

void UHeadMovementComponent::ServerSetTiltInput_Implementation(float Input)
{
	TiltInput = FMath::Clamp(Input, -1.f, 1.f);
}

void UHeadMovementComponent::ServerAddHeadRotation_Implementation(float DeltaYaw, float DeltaPitch)
{
	if (!IsStretched)
	{
		return;
	}
	
	HeadRotationValue.X = FMath::Clamp(
		HeadRotationValue.X + DeltaYaw * HeadRotationSpeed,
		-HeadYawLimit, HeadYawLimit);
	
	HeadRotationValue.Z = FMath::Clamp(
		HeadRotationValue.Z + DeltaPitch * HeadRotationSpeed,
		HeadPitchMin, HeadPitchMax);
}

FRotator UHeadMovementComponent::GetNeckRotation() const
{
	return FRotator::ZeroRotator;
}

FVector UHeadMovementComponent::GetNeckTranslation() const
{
	float Angle = 0.f;
	
	switch (StretchDirection)
	{
	case ENeckStretchDirection::Left:
		Angle = -DirectionAngle;
		break;
	case ENeckStretchDirection::Right:
		Angle = DirectionAngle;
		break;
	default:
		break;
	}
	
	const float Rad = FMath::DegreesToRadians(Angle);
	
	// X = 좌우 / Y = 정면 / Z = 위아래
	const float Forward = NeckReachDistance * FMath::Cos(Rad);
	const float Side = NeckReachDistance * FMath::Sin(Rad);
	
	return FVector(-Side, Forward, 0.f) * CurrentStretch;
}

FRotator UHeadMovementComponent::GetHeadRotation() const
{
	// Pitch(Y) = 기울이기 / Yaw(Z) = 좌우 / Roll(X) = 위아래
	return FRotator(CurrentTilt, HeadRotationValue.X, HeadRotationValue.Z);
}

void UHeadMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	// 목 늘리기 보간
	const float StretchTarget = IsStretched ? 1.f : 0.f;
	if (!FMath::IsNearlyEqual(CurrentStretch, StretchTarget))
	{
		const float Speed = (StretchDuration > 0.f) ? (1.f / StretchDuration) : 1.f;
		CurrentStretch = FMath::FInterpConstantTo(CurrentStretch, StretchTarget, DeltaTime, Speed);
	}
	
	// 기울이기
	const float TiltTarget = TiltInput * HeadRollLimit;
	
	if (TiltDuration <= 0.f)
	{
		CurrentTilt = TiltTarget;
	}
	else if (!FMath::IsNearlyEqual(CurrentTilt, TiltTarget))
	{
		const float TiltSpeed = HeadRollLimit / TiltDuration;
		CurrentTilt = FMath::FInterpConstantTo(CurrentTilt, TiltTarget, DeltaTime, TiltSpeed);
	}
}