// WeaponVFXComponent.cpp

#include "WeaponVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

UWeaponVFXComponent::UWeaponVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponVFXComponent::PlayFireEffects()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UStaticMeshComponent* WeaponMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVFXComponent] StaticMeshComponent not found on %s"), *Owner->GetName());
		return;
	}

	if (!WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVFXComponent] Socket '%s' not found on %s"), *MuzzleSocketName.ToString(), *Owner->GetName());
		return;
	}

	if (MuzzleFlashFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleFlashFX,
			WeaponMesh,
			MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	AWeaponBase* Weapon = Cast<AWeaponBase>(Owner);
	if (Weapon)
	{
		if (USoundBase** FoundSound = FireSounds.Find(Weapon->WeaponType))
		{
			if (*FoundSound)
			{
				const FVector SocketLoc = WeaponMesh->GetSocketLocation(MuzzleSocketName);
				UGameplayStatics::PlaySoundAtLocation(this, *FoundSound, SocketLoc);
			}
		}
	}
}
