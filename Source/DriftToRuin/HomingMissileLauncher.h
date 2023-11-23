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

	AActor* GetLastTarget() const;
	
	bool IsCharging();
	int32 GetChargeAmount();

	float GetChargeValue();
	float GetChargeCapValue();

	UFUNCTION(BlueprintImplementableEvent)
	void MissileFired(int32 ChargeNumber);
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownTime();

	UFUNCTION(BlueprintCallable)
	bool GetIsOnCooldown();
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeBuildUpRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeValue;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeValueCap;
	
	int32 ChargeAmount;
	
	bool bIsCharging;
	bool bIsOnCooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetingRange;
	
	UPROPERTY()
	AActor* CurrentTarget;

	UPROPERTY()
	AActor* LastTarget;
	
protected:
	virtual void BeginPlay() override;

private:
	void FindTarget();
	void ChargeFire();
	void OnChargeFire();
	void Fire();
	void OnFire();

	void CheckTargetVisibility();
	bool CheckTargetLineOfSight(const AController* Controller) const;
	bool CheckTargetInScreenBounds(const APlayerController* PlayerController) const;
	bool CheckTargetInRange(const ABaseVehiclePawn* VehicleOwner) const;

	void ResetCooldown();
public:
	virtual void Tick(float DeltaSeconds) override;
};
