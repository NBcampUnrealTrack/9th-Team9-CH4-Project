// WeaponVFXComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TabulletProject/WeaponBase.h"
#include "WeaponVFXComponent.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TABULLETPROJECT_API UWeaponVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponVFXComponent();

	// Fire 로직에서 호출할 함수 (클라이언트 로컬 재생용)
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void PlayFireEffects();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TMap<EWeaponType, USoundBase*> FireSounds;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	FName MuzzleSocketName = "Muzzle";
};