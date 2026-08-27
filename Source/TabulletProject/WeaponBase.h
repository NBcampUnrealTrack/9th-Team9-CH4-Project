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
	float SplashRadius;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	bool bIsInstantKill;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void InitializeWeaponData();

	// TODO: 서버 권위 처리 필요 - 현재는 로컬 테스트용 구조
	// 클라이언트가 부르는 진입점. 지금은 바로 라인트레이스를 실행하지만,
	// 최종적으로는 Server_Fire()를 호출하도록 바꿔야 함 (판정은 서버에서만).
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();

	// TODO: 아래 두 함수는 아직 미구현. 멀티플레이 붙일 때 작업 필요.
	// - Server_Fire(): 서버에서만 실제 라인트레이스 계산 (Reliable, WithValidation)
	// - Multicast_FireEffect(): 서버가 판정 후 모든 클라이언트에 이펙트/사운드 전파

	// UFUNCTION(Server, Reliable, WithValidation)
	// void Server_Fire();

	// UFUNCTION(NetMulticast, Reliable)
	// void Multicast_FireEffect(const FVector& Start, const FVector& End, bool bHit);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};