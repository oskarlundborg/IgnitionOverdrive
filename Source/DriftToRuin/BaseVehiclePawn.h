// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "WheeledVehiclePawn.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
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

	void InitVFX();

	void InitAudio();
	
	void UpdateGravelVFX() const;
	void UpdateAirbornePhysics() const;
	void UpdateEngineSFX() const;
	
	UPROPERTY(Category=DebugTools, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bPlayEngineSound = false;

	UPROPERTY(Category=DebugTools, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bUseCrazyCamera = false;

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

	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float DefaultBoostCost = 0.5f;

	//How often boost recharges.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostRechargeRate = 0.1f;

	//Boost Recharge amount per tick.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostRechargeAmount = 0.05f;
	
	//Max Torque when boosting.
	UPROPERTY(EditDefaultsOnly, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float BoostMaxTorque = 10000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float DefaultCameraLagMaxDistance = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BoostCameraLagMaxDistance = 150.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float DefaultCameraFOV = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BoostCameraFOV = 115.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BoostCameraInterpSpeed = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float BoostEndCameraInterpSpeed = 0.5f;

public:
	ABaseVehiclePawn();

	UPROPERTY(BlueprintReadWrite)
	int HeldPowerup = 0;

	UPROPERTY(Category=Powerup, EditAnywhere, BlueprintReadOnly)
	class UPowerupComponent* PowerupComponent;

	UPROPERTY(Category=Health, EditAnywhere, BlueprintReadOnly)
	class UHealthComponent* HealthComponent;

	UFUNCTION()
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void BeginPlay() override;

	void OnBoostPressed();
	void OnBoostReleased();
	void OnBoosting();

	void SetBoostCost(float NewBoostCost);
	void ResetBoostCost();
	
	void RechargeBoost();

	UFUNCTION(BlueprintCallable)
	void SetBoostAmount(float NewAmount);

	UFUNCTION(BlueprintCallable)
	float GetMaxBoostAmount();

	UFUNCTION(BlueprintPure)
	float GetBoostPercentage() const;

	UFUNCTION(BlueprintPure)
	bool GetIsBoostEnabled() const;

	UFUNCTION()
	bool IsGrounded() const;

	float GetMinigunDamage();
	float GetMinigunDefaultDamage();
	float GetHomingDamage();
	
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
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

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
	
	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* DirtVfxNiagaraComponentBLWheel;

	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* DirtVfxNiagaraComponentBRWheel;

	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* DirtVfxNiagaraComponentFLWheel;

	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraComponent* DirtVfxNiagaraComponentFRWheel;

	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* DirtVfxNiagaraSystem;

	UPROPERTY(Category=Sound, EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* EngineAudioSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<APlayerTurret> PlayerTurretClass;
	UPROPERTY()
	APlayerTurret* Turret;

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

	// ___
	// Front Bumper
	UPROPERTY(EditDefaultsOnly, Category="Bumper")
	UCapsuleComponent* BumperCollision;

	UPROPERTY(EditDefaultsOnly, Category="Bumper", meta=(AllowPrivateAccess=true))
	float BumperDamageDividedBy = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Bumper", meta=(AllowPrivateAccess=true))
	float MaxBumperDamage = 50.f;
};
