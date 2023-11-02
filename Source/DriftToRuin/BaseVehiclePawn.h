// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "BaseVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API ABaseVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	ABaseVehiclePawn();

	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

protected:
	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(Category=Camera, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

private:
	UChaosVehicleMovementComponent* VehicleMovementComponent;

	
	
};
