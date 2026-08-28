// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitReactionComponent.generated.h"

class UAnimMontage;
class UParticleSystem;
// class UNiagaraSystem; -> 나중에 나이아가라 쓴다고 하면 파티클 지우고 이거
class UHealthComponent;

// 사망 몽타주 재생 종료시 권한 회수
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathMontageFinished);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitReactionComponent();
	
	UPROPERTY(BlueprintAssignable, Category = "HitReaction")
	FOnDeathMontageFinished OnDeathMontageFinished;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth);
	
	UFUNCTION()
	void HandleDeathVisual();
	
	UFUNCTION()
	void OnDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	void PlayHitMontage();
	void SpawnHitEffect();
	
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	TObjectPtr<UAnimMontage> HitMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	TObjectPtr<UAnimMontage> DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	TObjectPtr<UParticleSystem> HitEffect;
	// TObjectPtr<UNiagaraSystem> HitEffect; -> 나이아가라 쓰면 쓰기
	
	float LastKnownHealth = 0.f;
};
