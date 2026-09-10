// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ViewModeComponent.generated.h"

class USpringArmComponent;
class UHeadMovementComponent;
class ATPGameState;
enum class ETabulletMatchPhase : uint8;

UENUM(BlueprintType)
enum class EViewMode : uint8
{
	FirstPerson, // 사격
	TopDown      // 알까기
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UViewModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UViewModeComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	
	// 시점 변경 페이즈 바뀔때 호출
	UFUNCTION(BlueprintCallable, Category = "ViewMode")
	void SetViewMode(EViewMode NewMode);
	
	UFUNCTION(BlueprintPure, Category = "ViewMode")
	EViewMode GetViewMode() const { return CurrentMode; }
	
	UFUNCTION(BlueprintPure, Category = "ViewMode")
	bool IsFirstPerson() const { return CurrentMode == EViewMode::FirstPerson; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void GetTargetValues(float& OutArmLength, FVector& OutOffset, float& OutPitch) const;
	void UpdateRotationSource();
	void UpdateViewModeFromPhase();

	void TryBindGameState();

	UPROPERTY()
	TObjectPtr<ATPGameState> GameStateRef;

	bool bBoundToGameState = false;

	// 사격 페이즈 "진입"을 감지하기 위한 마지막으로 본 MatchPhase (그 외엔 자동 전환 안 함)
	ETabulletMatchPhase LastCheckedMatchPhase = static_cast<ETabulletMatchPhase>(0); // WaitingForPlayers, 매치 시작 전 기본값

	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY()
	TObjectPtr<UHeadMovementComponent> HeadMovement;
	
	EViewMode CurrentMode = EViewMode::FirstPerson;
	bool Blending = false;
	
	// 1인칭
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|FirstPerson")
	float FirstPersonArmLength = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|FirstPerson")
	FVector FirstPersonOffset = FVector::ZeroVector;
	
	// 캐릭별 목 늘리기 수평 보정
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|FirstPerson")
	FRotator FirstPersonRotationOffset = FRotator::ZeroRotator;
	
	// 탑뷰
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|TopDown")
	float TopDownArmLength = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|TopDown")
	FVector TopDownOffset = FVector::ZeroVector;
	
	// 탑뷰에서 내려다보는 각도
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|TopDown")
	float TopDownPitch = -70.f;
	
	// 탑뷰 시점 고정
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode|TopDown")
	float TopDownYaw = 0.f;
	
	// 시점 변경 속도
	UPROPERTY(EditDefaultsOnly, Category = "ViewMode")
	float BlendSpeed = 5.f;
};
