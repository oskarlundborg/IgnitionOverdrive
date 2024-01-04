// Fill out your copyright notice in the Description page of Project Settings.
/**
* @author Mihajlo Radotic
*   Child weapon class used for the secondary weapon, homing missile launcher. 
*   Includes missile launcher specific components and functionality. 
*
* @author Daniel Olsson
*   Responsible for AI related code
**/
#pragma once

#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "BaseWeapon.h"
#include "HomingMissileLauncher.generated.h"

class APlayerVehiclePawn;
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

	void InitializeOwnerVariables();
	
	AActor* GetLastTarget() const;
	
	bool IsCharging();
	int32 GetChargeAmount();

	float GetChargeValue();
	float GetChargeCapValue();

	bool CheckTargetInRange(const ABaseVehiclePawn* VehicleOwner) const;
	bool CheckTargetLineOfSight(const AController* Controller, const AActor* Target) const;
	
	void OnFireAI(AActor* Target, int32 Charge);

	/*Blueprint implementable event for playing missile SFX when it spawns*/
	UFUNCTION(BlueprintImplementableEvent)
	void MissileFired(int32 ChargeNumber);
	
	UFUNCTION(BlueprintCallable)
	float GetCooldownTime();

	UFUNCTION(BlueprintCallable)
	bool GetIsOnCooldown();

	UFUNCTION(BlueprintCallable)
	bool GetCanLockOnTarget();

	UFUNCTION()
	void Fire(AActor* Target);

	float GetTargetRange();
	float GetChargeValueCap();
	FTimerHandle& GetFireTimer();
	void SetChargeAmount(float NewChargeAmount);
	void SetAICooldown();

	void DisableShooting();
	
private:
	UPROPERTY()
	const ABaseVehiclePawn* CarOwner;
	UPROPERTY()
	APlayerVehiclePawn* CarOwnerPlayer;
	/*UPROPERTY()
	const AController* OwnerController;
	UPROPERTY()
	const APlayerController* OwnerPlayerController;*/
	
	FTimerHandle ChargeHandle;
	FTimerHandle FireTimer;
	FTimerHandle CooldownTimer;
	FTimerHandle LockOnSoundTimer;

	UPROPERTY(Category = "Sound", EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAudioComponent* CanLockOnAudioComponent;
	bool bCanPlayLockOn;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	int32 ChargeCap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
    float ChargeTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float CooldownDuration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float CooldownOneCharge;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float CooldownTwoCharges;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float CooldownThreeCharges;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeBuildUpRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeValue;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float ChargeValueCap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge", meta = (AllowPrivateAccess = "true"))
	float TargetingOffset;
	
	int32 ChargeAmount;
	
	bool bIsCharging;
	bool bIsOnCooldown;
	bool bCanLockOn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetingRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float CloseRangeMagnitude;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float MidRangeMagnitude;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float FarRangeMagnitude;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float CloseTargetRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float MidTargetRange;
	
	UPROPERTY()
	AActor* CurrentTarget;
	UPROPERTY()
	AActor* LastTarget;
	
protected:
	virtual void BeginPlay() override;

private:
	
	void FindTarget();
	void CheckCanLockOn();
	bool PerformTargetLockLineTrace(FHitResult& HitResult);
	
	void ChargeFire();
	void OnChargeFire();
	UFUNCTION()
	void OnFire();

	void CheckTargetStatus();
	bool CheckTargetInScreenBounds(const APlayerController* PlayerController) const;
	bool CheckTargetIsDead(ABaseVehiclePawn* TargetVenchi) const;
	

	void ResetCooldown();
	void SetCooldownDuration();

	float GetValidMagnitude(AActor* Target);
	UFUNCTION()
	void SetTarget(ABaseProjectile* Projectile, ABaseVehiclePawn* Target);

	void PlayCanLockOnSound();
	void ResetCanLockOnSoundCooldown();

public:
	virtual void Tick(float DeltaSeconds) override;
};
