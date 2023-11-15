// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HomingMissileLauncher.generated.h"

class ABaseVehiclePawn;
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AHomingMissileLauncher : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AHomingMissileLauncher();

	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;

private:
	FTimerHandle TargetingHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	float AmmoCapacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	float AmmoAmount;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetingRange;

	UPROPERTY()
	ABaseVehiclePawn* CurrentTarget;
	

protected:
	virtual void BeginPlay() override;

private:
	void FindTarget();

public:
	virtual void Tick(float DeltaSeconds) override;
};
