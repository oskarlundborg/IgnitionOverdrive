// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTurret.h"
#include "AITurret.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AAITurret : public ABaseTurret
{
	GENERATED_BODY()

public:
	AAITurret();

protected:
	virtual void BeginPlay() override;

	/*Updates the rotation of the turret based on the controllers Yaw rotation*/
	virtual void UpdateTurretRotation() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
