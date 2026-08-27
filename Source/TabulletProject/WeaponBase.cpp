// WeaponBase.cpp

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// TODO : 지금은 로컬에서 바로 판정하는 임시 버전.
// TODO : 멀티플레이 단계에서는 이 안의 라인트레이스 로직을 Server_Fire()로 옮기고
// TODO : 여기서는 Server_Fire()를 호출하는 것으로 바꿔야 함

void AWeaponBase::Fire()
{
	if (!WeaponMesh)
	{
		return;
	}

	FVector StartLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
	FVector ForwardVector = WeaponMesh->GetSocketRotation(TEXT("MuzzleSocket")).Vector();
	FVector EndLocation = StartLocation + (ForwardVector * Range);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s hit %s with %s (Damage: %.1f)"),
				*GetName(), *HitActor->GetName(), *UEnum::GetValueAsString(WeaponType), Damage);

			// TODO : 실제 데미지 적용 로직 (UGameplayStatics::ApplyDamage 등)
			// TODO : 서버 권위 붙이면 이 블록 전체가 Server_Fire() 안으로 이동
		}
	}
}
