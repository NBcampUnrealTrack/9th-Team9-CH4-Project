// WeaponBase.cpp

#include "WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "TabulletProject/Component/AmmoComponent.h"
#include "TabulletProject/Component/HealthComponent.h"
#include "TabulletProject/Character/TPCharacter.h"
#include "TabulletProject/Component/WeaponVFXComponent.h"
#include "DrawDebugHelpers.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	
	VFXComponent = CreateDefaultSubobject<UWeaponVFXComponent>(TEXT("VFXComponent"));
}

void AWeaponBase::BeginPlay()
{
	InitializeWeaponData(); //총 생성되면 DataTable에서 스탯채움
	Super::BeginPlay();
}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeaponBase::InitializeWeaponData()
{
	if (!WeaponDataTable) return;

	FString RowNameStr = UEnum::GetValueAsString(WeaponType);
	RowNameStr.RemoveFromStart("EWeaponType::");

	FWeaponData* Row = WeaponDataTable->FindRow<FWeaponData>(FName(*RowNameStr), TEXT("WeaponDataLookup"));
	if (Row)
	{
		Damage = Row->Damage;
		Range = Row->Range;
		PelletCount = Row->PelletCount;
		SpreadAngle = Row->SpreadAngle;
		bIsInstantKill = Row->bIsInstantKill;
	}
}

void AWeaponBase::Fire()
{
	Server_Fire();
}

bool AWeaponBase::Server_Fire_Validate()
{
	return true;   // 필요시 나중에 검증 로직 추가
}

void AWeaponBase::Server_Fire_Implementation()
{
    if (!WeaponMesh) return;

    UAmmoComponent* AmmoComp = GetOwner() ? GetOwner()->FindComponentByClass<UAmmoComponent>() : nullptr;
    if (!AmmoComp || !AmmoComp->TryConsumeAmmo(WeaponType))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Weapon] 탄약 없음, 발사 취소: %s"), *UEnum::GetValueAsString(WeaponType));
        return;
    }
	// 모션 완성하면 주석 제거
    // FVector StartLocation = WeaponMesh->GetSocketLocation(TEXT("MuzzleSocket"));
    // FVector ForwardVector = WeaponMesh->GetSocketRotation(TEXT("MuzzleSocket")).Vector();
	
	// 통합 테스트용 임시 t포즈 사격
	ATPCharacter* OwnerCharacter = Cast<ATPCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	AController* OwnerController = OwnerCharacter->GetController();
	if (!OwnerController)
	{
		return;
	}

	FVector StartLocation;
	FRotator ViewRotation;

	OwnerController->GetPlayerViewPoint(StartLocation, ViewRotation);
	FVector ForwardVector = ViewRotation.Vector();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerCharacter); // 통합 테스트 t포즈 사격용

    int32 NumPellets = FMath::Max(PelletCount, 1);
    TSet<AActor*> HitActors;

    TArray<FVector> DebugStarts;
    TArray<FVector> DebugEnds;

    for (int32 i = 0; i < NumPellets; i++)
    {
        float AngleOffset = 0.0f;
        if (NumPellets > 1)
        {
            AngleOffset = -SpreadAngle * 0.5f + (SpreadAngle / (NumPellets - 1)) * i;
        }

        FVector PelletDirection = ForwardVector.RotateAngleAxis(AngleOffset, FVector::UpVector);
        FVector EndLocation = StartLocation + (PelletDirection * Range);

        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams);

        DebugStarts.Add(StartLocation);
        DebugEnds.Add(EndLocation);

        if (bHit)
        {
            AActor* HitActor = HitResult.GetActor();
            if (HitActor && !HitActors.Contains(HitActor))
            {
                HitActors.Add(HitActor);

                UE_LOG(LogTemp, Warning, TEXT("%s hit %s with %s (Damage: %.1d)"),
                    *GetName(), *HitActor->GetName(), *UEnum::GetValueAsString(WeaponType), Damage);

            	ATPCharacter* HitCharacter = Cast<ATPCharacter>(HitActor);
            	if (HitCharacter && HitCharacter->GetHealthComponent())
            	{
            		AController* KillerController = GetOwner() ? GetOwner()->GetInstigatorController() : nullptr;
            		HitCharacter->GetHealthComponent()->ApplyHealthDamage(Damage, KillerController);
            	}
            }
        }
    }

    Multicast_FireEffect(DebugStarts, DebugEnds);
}

void AWeaponBase::Multicast_FireEffect_Implementation(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints)
{
	for (int32 i = 0; i < StartPoints.Num(); i++)
	{
		DrawDebugLine(GetWorld(), StartPoints[i], EndPoints[i], FColor::Red, false, 1.0f, 0, 1.0f);
	}
	
	if (VFXComponent)          
	{
		VFXComponent->PlayFireEffects();
	}
}
