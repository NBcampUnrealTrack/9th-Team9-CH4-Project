// WeaponManagerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TabulletProject/WeaponType.h"
#include "WeaponManagerComponent.generated.h"

class AWeaponBase;
class UInputAction;
struct FInputActionValue;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TABULLETPROJECT_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponManagerComponent();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwitchWeapon(EWeaponType NewType);
	
	UFUNCTION(Server, Reliable)
	void ServerSwitchWeapon(EWeaponType NewType);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void FireCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFireCurrentWeapon();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponType)
	EWeaponType CurrentWeaponType;

	UFUNCTION()
	void OnRep_CurrentWeaponType();
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponArray)
	TArray<TObjectPtr<AWeaponBase>> ReplicatedWeapons;

	UFUNCTION()
	void OnRep_WeaponArray();

	void ApplyWeaponSwitch(EWeaponType NewType);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> RevolverClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> ShotgunClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> SniperClass;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("WeaponSocket");
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Weapon1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Weapon2Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Weapon3Action;

private:
	void SpawnAllWeapons();
	void TryBindInput();

	UFUNCTION()
	void OnWeapon1(const FInputActionValue& Value);

	UFUNCTION()
	void OnWeapon2(const FInputActionValue& Value);

	UFUNCTION()
	void OnWeapon3(const FInputActionValue& Value);

	UPROPERTY()
	TMap<EWeaponType, TObjectPtr<AWeaponBase>> WeaponInstances;

	UPROPERTY()
	TObjectPtr<AWeaponBase> CurrentWeapon;

	FTimerHandle BindRetryHandle;
};
