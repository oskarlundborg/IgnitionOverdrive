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

public:
	
	ABaseVehiclePawn();

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;
	
protected:
	
	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

protected:
	UPROPERTY(Category=Health, EditAnywhere, BlueprintReadOnly)
	class UHealthComponent* HealthComponent;
	
	//May be irrelevant, will be tested later.
	UPROPERTY(Category=Health, EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 100;

	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<APlayerTurret> PlayerTurretClass;
	UPROPERTY()
	APlayerTurret* Turret;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AMinigun> MinigunClass;
	UPROPERTY()
	AMinigun* Minigun;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<AHomingMissileLauncher> HomingLauncherClass;
	UPROPERTY()
	AHomingMissileLauncher* HomingLauncher;
	
};
