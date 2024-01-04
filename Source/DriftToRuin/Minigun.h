/**
* @author Mihajlo Radotic
*   Child weapon class used for the primary weapon, minigun. 
*   Includes minigun specific components and functionality. 
*
* @author Daniel Olsson
*   Responsible for AI related code
*   AIAdjustProjectileAimToCrosshair method
*
* @author Hugo Westgren
*   Responsible for powerup related code
**/

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Minigun.generated.h"

class APlayerVehiclePawn;
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

	/*Blueprint implementable event that stops shooting audio when called*/
	UFUNCTION(BlueprintImplementableEvent)
	void MinigunDisableAudio();
	
	UFUNCTION(BlueprintCallable)
	bool GetIsOverheated();

	void InitializeOwnerVariables();
	
	/*Blueprint implementable event for attaching trace VFX on projectiles, called when a projectile is spawned. Uses GetIsPoweredUp to attach the appropriate VFX type*/
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
	UPROPERTY()
	const APlayerVehiclePawn* CarOwner;
	//UPROPERTY()
	//const AController* OwnerController;
	
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

	UFUNCTION(BlueprintCallable)
	bool GetIsPoweredUp();

	UFUNCTION(BlueprintCallable)
	bool GetIsFiring();

	void DisableShooting();
	
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
