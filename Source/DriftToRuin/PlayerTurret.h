// Fill out your copyright notice in the Description page of Project Settings.

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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*Updates the rotation of the turret based on the controllers Yaw rotation*/
	virtual void UpdateTurretRotation() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
