// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AController*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathVisual);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TABULLETPROJECT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return Health <= 0.f; }
	
	// 서버 전용 실제 적용된 뎀 반환
	float ApplyHealthDamage(float DamageAmount, AController* Killer);
	
	// 서버 전용
	void Heal(float HealAmount);
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeathVisual OnDeathVisual;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, VisibleAnywhere, Category = "Health")
	float Health = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Replicated, Category = "Health")
	float MaxHealth = 10.f;
	
	UFUNCTION()
	void OnRep_Health();
};
