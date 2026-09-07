// WeaponVFXComponent.cpp

#include "WeaponVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

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

	USkeletalMeshComponent* WeaponMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WeaponVFXComponent] SkeletalMeshComponent not found on %s"), *Owner->GetName());
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
			true // bAutoDestroy
		);
	}

	if (FireSound)
	{
		const FVector SocketLoc = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, SocketLoc);
	}
}
