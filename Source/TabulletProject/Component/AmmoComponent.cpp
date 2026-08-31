// AmmoComponent.cpp

#include "AmmoComponent.h"
#include "Net/UnrealNetwork.h"
#include "TabulletProject/WeaponType.h"

UAmmoComponent::UAmmoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAmmoComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAmmoComponent, BasicAmmo);
	DOREPLIFETIME(UAmmoComponent, ShotgunAmmo);
	DOREPLIFETIME(UAmmoComponent, SniperAmmo);
}

int32* UAmmoComponent::GetAmmoRef(EWeaponType Type)
{
	switch (Type)
	{
	case EWeaponType::Revolver: return &BasicAmmo;
	case EWeaponType::Shotgun:  return &ShotgunAmmo;
	case EWeaponType::Sniper:   return &SniperAmmo;
	default: return nullptr;
	}
}

const int32* UAmmoComponent::GetAmmoRef(EWeaponType Type) const
{
	return const_cast<UAmmoComponent*>(this)->GetAmmoRef(Type);
}

void UAmmoComponent::SetAmmoCount(EWeaponType Type, int32 Count)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (int32* Ammo = GetAmmoRef(Type))
	{
		*Ammo = FMath::Max(0, Count);
		OnAmmoChanged.Broadcast(Type);
	}
}

bool UAmmoComponent::TryConsumeAmmo(EWeaponType Type)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Ammo] 소진됨! 타입: %d"), (int32)Type); // 디버깅용
		return false;
	}

	int32* Ammo = GetAmmoRef(Type);
	if (!Ammo || *Ammo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Ammo] 소비됨! 타입: %d, 남은 개수: %d"), (int32)Type, *Ammo); // 디버깅용
		return false;
	}

	(*Ammo)--;
	OnAmmoChanged.Broadcast(Type);
	return true;
}

int32 UAmmoComponent::GetAmmoCount(EWeaponType Type) const
{
	const int32* Ammo = GetAmmoRef(Type);
	return Ammo ? *Ammo : 0;
}

void UAmmoComponent::OnRep_BasicAmmo()
{
	OnAmmoChanged.Broadcast(EWeaponType::Revolver);
}

void UAmmoComponent::OnRep_ShotgunAmmo()
{
	OnAmmoChanged.Broadcast(EWeaponType::Shotgun);
}

void UAmmoComponent::OnRep_SniperAmmo()
{
	OnAmmoChanged.Broadcast(EWeaponType::Sniper);
}

