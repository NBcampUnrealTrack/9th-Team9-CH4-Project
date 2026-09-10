// WeaponManagerComponent.cpp

#include "WeaponManagerComponent.h"
#include "TabulletProject/WeaponBase.h"
#include "TabulletProject/TPGameMode.h"
#include "TabulletProject/Component/AmmoComponent.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

UWeaponManagerComponent::UWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnAllWeapons();
	TryBindInput();
}

void UWeaponManagerComponent::SpawnAllWeapons()
{
	ACharacter* OwnerCharacter = GetOwner<ACharacter>();
	if (!OwnerCharacter) return;

	auto SpawnAndAttach = [&](TSubclassOf<AWeaponBase> Class, EWeaponType Type)
	{
		if (!Class) return;
		
		if (!OwnerCharacter->HasAuthority()) return; //서버만 스폰

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;

		AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(Class, SpawnParams);
		if (NewWeapon)
		{
			NewWeapon->AttachToComponent(OwnerCharacter->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
			NewWeapon->SetActorHiddenInGame(true);
			WeaponInstances.Add(Type, NewWeapon);
			ReplicatedWeapons.Add(NewWeapon);
		}
	};

	SpawnAndAttach(RevolverClass, EWeaponType::Revolver);
	SpawnAndAttach(ShotgunClass, EWeaponType::Shotgun);
	SpawnAndAttach(SniperClass, EWeaponType::Sniper);

	// 시작 무기: 리볼버
	SwitchWeapon(EWeaponType::Revolver);
}

void UWeaponManagerComponent::SwitchWeapon(EWeaponType NewType)
{
	ServerSwitchWeapon(NewType);
}

void UWeaponManagerComponent::ServerSwitchWeapon_Implementation(EWeaponType NewType)
{
	CurrentWeaponType = NewType;
	ApplyWeaponSwitch(NewType);
}

void UWeaponManagerComponent::OnRep_CurrentWeaponType()
{
	ApplyWeaponSwitch(CurrentWeaponType);
}

void UWeaponManagerComponent::OnRep_WeaponArray()
{
	WeaponInstances.Empty();
	for (AWeaponBase* Weapon : ReplicatedWeapons)
	{
		if (Weapon)
		{
			WeaponInstances.Add(Weapon->WeaponType, Weapon);
		}
	}
	ApplyWeaponSwitch(CurrentWeaponType);
}

void UWeaponManagerComponent::ApplyWeaponSwitch(EWeaponType NewType)
{
	TObjectPtr<AWeaponBase>* Found = WeaponInstances.Find(NewType);
	if (!Found || !*Found) return;

	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}

	CurrentWeapon = *Found;
	CurrentWeapon->SetActorHiddenInGame(false);
}

void UWeaponManagerComponent::FireCurrentWeapon()
{
	ServerFireCurrentWeapon();
}

void UWeaponManagerComponent::ServerFireCurrentWeapon_Implementation()
{
	APawn* OwnerPawn = GetOwner<APawn>();
	if (!OwnerPawn || !OwnerPawn->HasAuthority() || !CurrentWeapon)
	{
		return;
	}

	const UAmmoComponent* AmmoComponent = OwnerPawn->FindComponentByClass<UAmmoComponent>();
	if (!AmmoComponent || !AmmoComponent->HasAmmo(CurrentWeaponType))
	{
		return;
	}

	CurrentWeapon->Fire();

	if (ATPGameMode* TPGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATPGameMode>() : nullptr)
	{
		TPGameMode->NotifyShotResolved(OwnerPawn->GetController());
	}
}

void UWeaponManagerComponent::TryBindInput()
{
	APawn* OwnerPawn = GetOwner<APawn>();
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent))
		{
			if (Weapon1Action) EIC->BindAction(Weapon1Action, ETriggerEvent::Triggered, this, &UWeaponManagerComponent::OnWeapon1);
			if (Weapon2Action) EIC->BindAction(Weapon2Action, ETriggerEvent::Triggered, this, &UWeaponManagerComponent::OnWeapon2);
			if (Weapon3Action) EIC->BindAction(Weapon3Action, ETriggerEvent::Triggered, this, &UWeaponManagerComponent::OnWeapon3);
			return;
		}
	}

	// InputComponent가 아직 준비 안 됐으면 재시도
	GetWorld()->GetTimerManager().SetTimer(BindRetryHandle, this, &UWeaponManagerComponent::TryBindInput, 0.2f, false);
}

void UWeaponManagerComponent::OnWeapon1(const FInputActionValue& Value) { SwitchWeapon(EWeaponType::Revolver); }
void UWeaponManagerComponent::OnWeapon2(const FInputActionValue& Value) { SwitchWeapon(EWeaponType::Shotgun); }
void UWeaponManagerComponent::OnWeapon3(const FInputActionValue& Value) { SwitchWeapon(EWeaponType::Sniper); }

void UWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponManagerComponent, CurrentWeaponType);
	DOREPLIFETIME(UWeaponManagerComponent, ReplicatedWeapons);
}

