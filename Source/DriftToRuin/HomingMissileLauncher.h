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

	UFUNCTION(BlueprintCallable)
	void ResetAmmo();

	UFUNCTION(BlueprintCallable)
	void SetAmmo(int32 Amount);

	UFUNCTION(BlueprintCallable)
	int32 GetAmmo();

private:
	FTimerHandle ChargeHandle;
	FTimerHandle FireTimer;
	FTimerHandle CooldownTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	int32 ChargeCap;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	//int32 AmmoAmount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
    float ChargeTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float CooldownDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetingRange;

	int32 ChargeAmount;

	bool bIsCharging;
	bool bIsOnCooldown;

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

	void CheckTargetVisibility();
	bool CheckTargetLineOfSight(AController* Controller);
	bool CheckTargetInScreenBounds(APlayerController* PlayerController);

	void ResetCooldown();

public:
	virtual void Tick(float DeltaSeconds) override;
};
