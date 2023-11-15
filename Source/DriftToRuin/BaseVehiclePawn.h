// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "BaseVehiclePawn.generated.h"
/*Maybe should be moved to player and AI classes, should work for first playable for now*/
class APlayerTurret;
class AHomingMissileLauncher;
class AMinigun;
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API ABaseVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()
	
	UPROPERTY(Category=DebugTools, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bPlayEngineSound = false;

	FTimerHandle BoostTimer;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boost", meta = (AllowPrivateAccess = "true"))
	float ConsumptionRate = 0.1f;

public:
	
	ABaseVehiclePawn();

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	struct 
	{
		bool bEnabled = false;
		float MaxBoostAmount = 100; //Max possible boost amount.
		float BoostAmount = 10000; //Initial boost amount.
		float BoostTorque = 10000.0f; //Max torque when boosting.
		float DefaultTorque = 800.0f; //Default max torque on vehicle.
		void SetBoostAmount(const float AmountToSet)
		{
			BoostAmount=AmountToSet;
		}
		void SetEnabled(const bool Enabled)
		{
			bEnabled=Enabled;
		}
	} Booster;

	void OnBoostPressed();
	void OnBoostReleased();
	void OnBoosting();

	float GetDamage();

	UFUNCTION(BlueprintImplementableEvent)
	void BoostStartEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void BoostStopEvent();
	
	UFUNCTION(BlueprintCallable)
	void SetDamage(float NewDamage);

	UFUNCTION(BlueprintCallable)
	void ApplyDamageBoost(float NewDamage, float TimerDuration);

	UFUNCTION()
	void RemoveDamageBoost(float OriginalDamage);
	
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
	float Damage = 5;
	
	UPROPERTY(Category=Sound, EditDefaultsOnly, BlueprintReadOnly)
	UAudioComponent* EngineAudioComponent;

	UPROPERTY(Category=Sound, EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* EngineAudioSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<APlayerTurret> PlayerTurretClass;
	UPROPERTY()
	APlayerTurret* Turret;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AMinigun> MinigunClass;
	UPROPERTY()
	AMinigun* Minigun;

public:
	APlayerTurret* GetTurret() const;
	AMinigun* GetMinigun() const;
	AHomingMissileLauncher* GetHomingLauncher() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AHomingMissileLauncher> HomingLauncherClass;
	UPROPERTY()
	AHomingMissileLauncher* HomingLauncher;

	//timer för att ta bort damage-effekten
	FTimerHandle DamageBoostTimerHandle;
};
