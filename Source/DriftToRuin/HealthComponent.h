// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRIFTTORUIN_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void DamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* Instigator, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable)
	void SetHealth(float NewHealth);

	UFUNCTION(BlueprintCallable)
	bool IsDead() const;

	//Is preferably called upon in the constructor of any object using the HealthComponent.
	void SetMaxHealth(float NewHealth);
	
private:
	UPROPERTY(Category=Health, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	float CurrentHealth = 0.f;

	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	float MaxHealth = 100;
		
};
