// Fill out your copyright notice in the Description page of Project Settings.


#include "TableFallJudgeComponent.h"
#include "TabulletProject/Table/Actors/TableBulletPiece.h"

// Sets default values for this component's properties
UTableFallJudgeComponent::UTableFallJudgeComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// 콜리전 없이 겹치는 것만 감지
	SetGenerateOverlapEvents(true);

	SetCollisionResponseToAllChannels(ECR_Ignore);					// 기본적으로 모든 물체 무시
	SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);		// 물리 물체만 감지

	InitBoxExtent(FVector(300.0, 300.0, 50.0));			// 생성자용 초기 크기 설정
}

void UTableFallJudgeComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddUniqueDynamic(
		this,
		&UTableFallJudgeComponent::HandleBeginOverlap);
}

bool UTableFallJudgeComponent::HasPassedBelowDetectionPlane(const FVector& WorldLocation) const
{
	const FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	return LocalLocation.Z <= GetUnscaledBoxExtent().Z;
}

void UTableFallJudgeComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())		// 서버에서만 판정하게
	{
		return;
	}

	ATableBulletPiece* FallenPiece =
		Cast<ATableBulletPiece>(OtherActor);			// 총알만 처리함

	if (!IsValid(FallenPiece))
	{
		return;
	}

	OnPieceEnteredFallJudge.Broadcast(FallenPiece);		// 총알 낙하 알려주는 이벤트
}
