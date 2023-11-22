// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "BaseVehiclePawn.generated.h"
/*Maybe should be moved to player and AI classes, should work for first playable for now*/
class APlayerTurret;
class AHomingMissileLauncher;
class AMinigun;

USTRUCT()
struct FBooster
{
	GENERATED_BODY()

	FTimerHandle BoostTimer;
	FTimerHandle RechargeTimer;
	
	bool bEnabled = false;
	float MaxBoostAmount = 100; //Max possible boost amount.
	float BoostAmount = 100; //Initial boost amount.
	float DefaultTorque = 800.0f; //Default max torque on vehicle.

	void SetEnabled(const bool Enabled)
	{
		bEnabled=Enabled;
	}
};
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API ABaseVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()
	
	UPROPERTY(Category=DebugTools, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bPlayEngineSound = false;

	UPROPERTY(EditDefaultsOnly, Category = "Physics", meta = (AllowPrivateAccess = "true"))
	float AirborneDownforceCoefficient = 2.5f;
	
	//Struct for booster
	UPROPERTY()
	FBooster Booster;
	
	//How often boost is consumed.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostConsumptionRate = 1.0f;

	//Amount of boost consumed per call (BoostConsumptionRate).
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostCost = 0.5f;

	//How often boost recharges.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostRechargeRate = 0.1f;

	//Boost Recharge amount per tick.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostRechargeAmount = 0.05f;
	
	//Max Torque when boosting.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostMaxTorque = 10000.0f;

public:
	ABaseVehiclePawn();

	UPROPERTY(BlueprintReadWrite)
	int HeldPowerup = 0;

	UPROPERTY(Category=Powerup, EditAnywhere, BlueprintReadOnly)
	class UPowerupComponent* PowerupComponent;

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void BeginPlay() override;

	void OnBoostPressed();
	void OnBoostReleased();
	void OnBoosting();
	
	void RechargeBoost();

	UFUNCTION(BlueprintCallable)
	void SetBoostAmount(float NewAmount);

	UFUNCTION(BlueprintPure)
	float GetBoostPercentage() const;

	UFUNCTION(BlueprintPure)
	bool GetIsBoostEnabled() const;

	UFUNCTION()
	bool IsGrounded() const;

	float GetMinigunDamage();
	float GetMinigunDefaultDamage();
	float GetHomingDamage();

	UFUNCTION(BlueprintImplementableEvent)
	void BoostStartEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void BoostStopEvent();
	
	UFUNCTION(BlueprintCallable)
	void SetDamage(float NewDamage);

	UFUNCTION()
	void SetMinigunDamage(int NewDamage);


	UFUNCTION(BlueprintCallable)
	void ApplyDamageBoost(float NewDamage, float TimerDuration);

	UFUNCTION()
	void RemoveDamageBoost(float OriginalDamage);

	bool GetIsDead();

	UFUNCTION()
	void OnBumperBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)
	APlayerTurret* GetTurret() const;
	UFUNCTION(BlueprintCallable)
	AMinigun* GetMinigun() const;
	UFUNCTION(BlueprintCallable)
	AHomingMissileLauncher* GetHomingLauncher() const;

	UFUNCTION(BlueprintCallable)
	float GetScrapPercentage();
	UFUNCTION(BlueprintCallable)
	void AddScrapAmount(float Scrap, float HealAmount);
	UFUNCTION(BlueprintCallable)
	void RemoveScrapAmount(float Scrap);
	UFUNCTION(BlueprintCallable)
	float GetScrapToDrop();
	UFUNCTION(BlueprintCallable)
	int GetKillpointWorth();
	UFUNCTION(BlueprintCallable)
	void ResetScrapLevel();

	UFUNCTION(BlueprintCallable)
	void ActivatePowerup();
	UFUNCTION(BlueprintCallable)
	void SetHeldPowerup (int PowerIndex);

	UFUNCTION()
	void CheckScrapLevel();

	USceneComponent* GetHomingTargetPoint() const;
	
protected:
	UPROPERTY(Category=Components, EditDefaultsOnly, BlueprintReadOnly)
	class UChaosWheeledVehicleMovementComponent* VehicleMovementComp;
	
	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;
	
	UPROPERTY(Category=Health, EditAnywhere, BlueprintReadOnly)
	class UHealthComponent* HealthComponent;
	
	//May be irrelevant, will be tested later.
	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 100;

	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float MinigunDamage = 5;

	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float HomingDamage = 20.f;

	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float DefaultMinigunDamage = 5;

	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float DefaultHomingDamage = 20.f;
	
	UPROPERTY(Category=Sound, EditDefaultsOnly, BlueprintReadOnly)
	UAudioComponent* EngineAudioComponent;

	UPROPERTY(Category=Boost, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraComponent* BoostVfxNiagaraComponent;

	UPROPERTY(Category=Boost, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* BoostVfxNiagaraSystem;

	UPROPERTY(Category=Sound, EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* EngineAudioSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<APlayerTurret> PlayerTurretClass;
	UPROPERTY()
	APlayerTurret* Turret;

	UPROPERTY(EditDefaultsOnly)
	UBoxComponent* BumperCollisionBox;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	float DamageMultiplier = .004f;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	float BumperDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	bool bFlatDamage = false;

	float ScrapAmount = 0;
	float ScrapToDrop = 10;

	UPROPERTY(EditDefaultsOnly)
	float MaxScrap = 100;

	int KillpointWorth = 1;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	bool bHitLevelOne = false;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	bool bHitLevelTwo = false;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	bool bHitLevelThree = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AMinigun> MinigunClass;
	
	UPROPERTY()
	AMinigun* Minigun;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* HomingTargetPoint;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AHomingMissileLauncher> HomingLauncherClass;
	
	UPROPERTY()
	AHomingMissileLauncher* HomingLauncher;

	//timer för att ta bort damage-effekten
	FTimerHandle DamageBoostTimerHandle;
};
