// Fill out your copyright notice in the Description page of Project Settings.


#include "TableBulletPiece.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ATableBulletPiece::ATableBulletPiece()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;		// 복제
	SetReplicateMovement(true);		// 움직임 복제
	
	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PieceMesh"));
	
	SetRootComponent(PieceMesh);
	
	PieceMesh->SetMobility(EComponentMobility::Movable);		// 물리 이동 허용
	PieceMesh->SetSimulatePhysics(true);			// 카오스 물리 적용
	PieceMesh->SetEnableGravity(true);				// 테이블 밖으로 나가면 낙하
	PieceMesh->SetCollisionProfileName(TEXT("PhysicsActor"));		// 물리 충돌 설정
	
	PieceMesh->BodyInstance.bUseCCD = true;			// 빠르게 움직일 때 관통할 확률을 줄인다는데 모르겠음..
}

//  여기부터
void ATableBulletPiece::MarkAsOut()
{
	if (!HasAuthority() || IsOut())
	{
		return;
	}

	PieceState = ETablePieceState::Out;
	ApplyOutState();

	ForceNetUpdate();
}

void ATableBulletPiece::OnRep_PieceState()
{
	if (IsOut())
	{
		ApplyOutState();
	}
}

void ATableBulletPiece::ApplyOutState()
{
	PieceMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
	PieceMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	PieceMesh->SetSimulatePhysics(false);
	PieceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorHiddenInGame(true);
}

void ATableBulletPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATableBulletPiece, PieceType);
	DOREPLIFETIME(ATableBulletPiece, PieceState);
	DOREPLIFETIME(ATableBulletPiece, OwningPlayerState);
}
// 여기까지 서버관련 복제, 탈락한 총알 제외

bool ATableBulletPiece::ApplyFlickImpulse(const FVector& WorldImpulse)
{
	if (!HasAuthority())									//서버에서만 물리 작용
	{
		return false;
	}

	if (IsOut())											// 탈락된 총알은 판정에서 제외
	{
		return false;
	}

	if (WorldImpulse.IsNearlyZero())						// 힘이 0일 때 제외
	{
		return false;
	}

	if (!PieceMesh->IsSimulatingPhysics())					// 물리 시뮬레이션이 없는 총알은 제외
	{
		return false;
	}

	PieceMesh->WakeAllRigidBodies();						// 자고있는? 객체는 깨워서
	PieceMesh->AddImpulse(WorldImpulse);					// 임펄스 적용

	ForceNetUpdate();										

	return true;
}

bool ATableBulletPiece::IsMoving(float LinearThreshold, float AngularThreshold) const	// 아웃, 속도, 정지 판별
{
	if (IsOut() || !IsValid(PieceMesh) || !PieceMesh->IsSimulatingPhysics())
	{
		return false;
	}

	const float LinearSpeedSquared = PieceMesh->GetPhysicsLinearVelocity().SizeSquared();
	const float AngularSpeedSquared = PieceMesh->GetPhysicsAngularVelocityInDegrees().SizeSquared();

	return LinearSpeedSquared > FMath::Square(LinearThreshold) || AngularSpeedSquared > FMath::Square(AngularThreshold);
}

void ATableBulletPiece::SetOwningPlayerState(APlayerState* InOwningPlayerState)	// 서버에서 총알 소유자 정함
{
	if (!HasAuthority() || PieceType != ETablePieceType::Normal)
	{
		return;
	}

	OwningPlayerState = InOwningPlayerState;
	ForceNetUpdate();
}

APlayerState* ATableBulletPiece::GetOwningPlayerState() const				// 현재 소유자 누군지
{
	return OwningPlayerState;
}

bool ATableBulletPiece::IsOwnedByPlayerState(const APlayerState* PlayerState) const
{
	return IsValid(PlayerState) && OwningPlayerState == PlayerState;
}

