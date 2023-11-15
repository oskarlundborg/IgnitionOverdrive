// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AEnemyVehiclePawn : public ABaseVehiclePawn
{
	GENERATED_BODY()
	
public:

	AEnemyVehiclePawn();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	
private:
	float ThrottleInput;
	float BrakeInput;
	float SteeringInput; 
	
};
