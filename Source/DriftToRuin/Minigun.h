// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Components/PoseableMeshComponent.h"
#include "Minigun.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AMinigun : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AMinigun();

	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;
	bool GetIsOverheated();

	UFUNCTION(BlueprintImplementableEvent)
	void ProjectileSpawned(ABaseProjectile* Projectile);

	//Ammo för Miniguns powerup
	float PowerAmmo = 0;
	UPROPERTY(BlueprintReadWrite)
	float MaxPowerAmmo = 100;

protected:
	virtual void BeginPlay() override;

private:
	void BuildUpOverheat();
	void CoolDownWeapon();
	void OverheatCooldown();
	void UpdateOverheat();

	
	void AdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation);
	void AIAdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation);

	void Fire();
	void OnPullTrigger();

public:
	virtual void Tick(float DeltaSeconds) override;

private:
	FTimerHandle FireRateTimer;

	UPROPERTY()
	TArray<AActor*> ToIgnore;
	/*Controlls weapons FireRate*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	float FireRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	bool bIsFiring;

	/*Projectile crosshair aim variables*/
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance;

	/*Overheat logic variables*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	bool bIsOverheated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	float OverheatValue;

public:
	float GetOverheatValue() const;
	float GetOverheatMaxValue() const;
	
	UFUNCTION(BlueprintCallable)
	float GetPowerAmmoPercent();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Overheat")
	bool PoweredUp = false;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	float OverheatMax;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	float OverheatBuildUpRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	float OverheatCoolDownRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Overheat", meta = (AllowPrivateAccess = "true"))
	float OverheatCooldownDuration;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (AllowPrivateAccess = "true"))
	float ProjSpreadMinY;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (AllowPrivateAccess = "true"))
	float ProjSpreadMaxY;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (AllowPrivateAccess = "true"))
	float ProjSpreadMinZ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (AllowPrivateAccess = "true"))
	float ProjSpreadMaxZ;
};
