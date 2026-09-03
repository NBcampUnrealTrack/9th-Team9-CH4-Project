// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HeadMovementComponent.generated.h"

UENUM(BlueprintType)
enum class ENeckStretchDirection : uint8
{
	Left,		// 좌측 대각선
	Center,		// 정면
	Right		// 우측 대각선
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UHeadMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHeadMovementComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 방향 -> 다시 누르면 목 원복
	UFUNCTION(BlueprintCallable, Category = "HeadMovement")
	void ToggleStretch(ENeckStretchDirection Direction);
	
	// 마우스 -> 목이 늘어난 상태에서만
	UFUNCTION(BlueprintCallable, Category = "HeadMovement")
	void AddHeadRotation(float DeltaYaw, float DeltaPitch);
	
	// 기울이기 (-1 좌 / 0 / 우 +1) 
	UFUNCTION(BlueprintCallable, Category = "HeadMovement")
	void SetTiltInput(float Input);
	
	UFUNCTION(BlueprintCallable, Category = "HeadMovement")
	void ResetStretch();
	
	// 늘어난 상태인지 캐릭터가 마우스 입력을 어디로 보낼지
	UFUNCTION(BlueprintPure, Category = "HeadMovement")
	bool GetIsStretched() const { return IsStretched; }
	
	// 목 늘어난 정도 0~1
	UFUNCTION(BlueprintPure, Category = "HeadMovement")
	float GetStretchAlpha() const { return CurrentStretch; }
	
	// neck_01 rotation
	UFUNCTION(BlueprintPure, Category = "HeadMovement")
	FRotator GetNeckRotation() const;
	
	UFUNCTION(BlueprintPure, Category = "HeadMovement")
	FVector GetNeckTranslation() const;
	
	// head Rotation
	UFUNCTION(BlueprintPure, Category = "HeadMovement")
	FRotator GetHeadRotation() const;

protected:
	UFUNCTION(Server, Reliable)
	void ServerToggleStretch(ENeckStretchDirection Direction);
	
	UFUNCTION(Server, Unreliable)
	void ServerAddHeadRotation(float DeltaYaw, float DeltaPitch);
	
	UFUNCTION(Server, Reliable)
	void ServerSetTiltInput(float Input);
	
	UFUNCTION(Server, Reliable)
	void ServerResetStretch();
	
	// 서버에서만 사용하는 입력 상태
	float TiltInput = 0.f;
	
	// 마지막으로 서버에 보낸 값
	float LastSentTiltInput = 0.f;
	
	// 기울이기 각도
	UPROPERTY(Replicated)
	float CurrentTilt = 0.f;
	
	UPROPERTY(Replicated)
	float CurrentStretch = 0.f;
	
	UPROPERTY(Replicated)
	bool IsStretched = false;
	
	UPROPERTY(Replicated)
	ENeckStretchDirection StretchDirection = ENeckStretchDirection::Center;
	
	// X = 좌우 / Z = 위아래 (Y 안 씀, 기울이기는 CurrentTilt)
	UPROPERTY(Replicated)
	FVector HeadRotationValue = FVector::ZeroVector;
	
	// 최대 길이까지 걸리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement")
	float StretchDuration = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Neck")
	float NeckReachDistance = 30.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Neck")
	float DirectionAngle = 45.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float HeadYawLimit = 45.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float HeadRollLimit = 15.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float HeadPitchMin = -25.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float HeadPitchMax = 25.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float HeadRotationSpeed = 1.f;
	
	// 최대 각도까지 걸리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "HeadMovement|Head")
	float TiltDuration = 0.1f;
};
