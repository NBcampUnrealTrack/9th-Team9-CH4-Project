// WeaponBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h"
#include "WeaponBase.generated.h"

UCLASS()
class TABULLETPROJECT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	UDataTable* WeaponDataTable;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 Damage;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float Range;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	int32 PelletCount;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	float SpreadAngle;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bIsInstantKill;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitializeWeaponData();
	
	// 클라이언트가 부르는 진입점
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireEffect(const TArray<FVector>& StartPoints, const TArray<FVector>& EndPoints);
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};