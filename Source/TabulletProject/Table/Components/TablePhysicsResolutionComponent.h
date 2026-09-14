#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TablePhysicsResolutionComponent.generated.h"

class ATableBulletPiece;
class UTableFallJudgeComponent;

DECLARE_MULTICAST_DELEGATE(FOnTablePhysicsSettled);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTablePhysicsMissedFall, ATableBulletPiece*);

UCLASS(ClassGroup = (Table), meta = (BlueprintSpawnableComponent))
class TABULLETPROJECT_API UTablePhysicsResolutionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTablePhysicsResolutionComponent();

	void StartMonitoring(const TArray<TObjectPtr<ATableBulletPiece>>& Pieces, UTableFallJudgeComponent* FallJudge, float InLinearSpeedThreshold, float InAngularSpeedThreshold, float InRequiredSettledTime);
	void ResetMonitoring();
	void ForceFinish();

	bool IsResolving() const { return bMonitoringMovement; }
	bool AreAllPiecesSettled() const { return !bMonitoringMovement; }

	FOnTablePhysicsSettled OnPhysicsSettled;
	FOnTablePhysicsMissedFall OnMissedFall;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ATableBulletPiece>> MonitoredPieces;

	UPROPERTY(Transient)
	TObjectPtr<UTableFallJudgeComponent> FallJudge;

	float LinearSpeedThreshold = 10.0f;
	float AngularSpeedThreshold = 10.0f;
	float RequiredSettledTime = 0.5f;
	float SettledElapsedTime = 0.0f;
	bool bMonitoringMovement = false;
};
