// Fill out your copyright notice in the Description page of Project Settings.
/**
* @author Mihajlo Radotic
*   Child turret class used for players.
*   Empty as the rotation logic was moved to an animation blueprint (check ABP_ArmoredVehicle Event Graph for rotation logic)
**/
#pragma once

#include "CoreMinimal.h"
#include "BaseTurret.h"
#include "PlayerTurret.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API APlayerTurret : public ABaseTurret
{
	GENERATED_BODY()

public:
	APlayerTurret();

protected:
	virtual void BeginPlay() override;

	/*Updates the rotation of the turret based on the controllers Yaw rotation*/
	virtual void UpdateTurretRotation() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
