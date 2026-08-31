// AmmoComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TabulletProject/WeaponType.h"
#include "AmmoComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoChanged, EWeaponType, ChangedType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TABULLETPROJECT_API UAmmoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAmmoComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ammo")
	FOnAmmoChanged OnAmmoChanged;

	// 알까기 페이즈 종료 후 확보한 개수로 세팅 (서버 전용)
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	void SetAmmoCount(EWeaponType Type, int32 Count);

	// 발사 시 호출 - 탄약 있으면 1발 차감하고 true 반환 (서버 전용)
	UFUNCTION(BlueprintCallable, Category = "Ammo")
	bool TryConsumeAmmo(EWeaponType Type);

	UFUNCTION(BlueprintPure, Category = "Ammo")
	int32 GetAmmoCount(EWeaponType Type) const;

	UFUNCTION(BlueprintPure, Category = "Ammo")
	bool HasAmmo(EWeaponType Type) const { return GetAmmoCount(Type) > 0; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_BasicAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 BasicAmmo = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ShotgunAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 ShotgunAmmo = 0;

	UPROPERTY(ReplicatedUsing = OnRep_SniperAmmo, VisibleAnywhere, BlueprintReadOnly, Category = "Ammo")
	int32 SniperAmmo = 0;

	UFUNCTION()
	void OnRep_BasicAmmo();

	UFUNCTION()
	void OnRep_ShotgunAmmo();

	UFUNCTION()
	void OnRep_SniperAmmo();

	// 타입에 맞는 필드 포인터 반환
	int32* GetAmmoRef(EWeaponType Type);
	const int32* GetAmmoRef(EWeaponType Type) const;
};
