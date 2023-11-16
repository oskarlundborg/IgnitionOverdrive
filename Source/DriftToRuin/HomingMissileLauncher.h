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

	bool IsCharging();
	int32 GetChargeAmount();

	void ResetAmmo();
	void SetAmmo(int32 Amount);
	int32 GetAmmo();

private:
	FTimerHandle ChargeHandle;
	FTimerHandle FireTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	int32 AmmoCapacity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	int32 AmmoAmount;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetingRange;

	int32 ChargeAmount;

	bool bIsCharging;

	UPROPERTY()
	AActor* CurrentTarget;
	

protected:
	virtual void BeginPlay() override;

private:
	void FindTarget();
	void ChargeFire();
	void OnChargeFire();
	void Fire();
	void OnFire();

public:
	virtual void Tick(float DeltaSeconds) override;
};
