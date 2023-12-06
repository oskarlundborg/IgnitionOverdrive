// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.generated.h"

class AAITurret;
class UBehaviorTreeComponent;
class AAIController;
class UBlackboardComponent;
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
	void RandomlyRotateTurret();
	void ManageSpeed();
	void DriveAlongSpline();
	void CheckIfAtEndOfSpline();

	//set functions
	void SetSwitchString(const FString& NewSwitchString);
	void SetHasNewSplineBeenSetup(bool bValue);


	//pathfinding
	UPROPERTY(EditAnywhere)
	float SplineEndPointDistanceThreshold = 800;
	UPROPERTY(EditAnywhere)
	float NextPointOnSplineThreshold = 800;

	// car driving
	UPROPERTY(EditAnywhere)
	float ThrottleInput;
	UPROPERTY(EditAnywhere)
	float BrakeInput;
	UPROPERTY(EditAnywhere)
	float SteeringInput;
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1500.0f;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AAITurret> AITurretClass;
	UPROPERTY()
	AAITurret* Turret;

	FString SwitchString = "Drive";

	//Common Components
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	class USplineComponent* MySpline = nullptr;
	class UChaosVehicleMovementComponent* VehicleMovementComponent = nullptr;

	//spline values
	FVector Destination = FVector::ZeroVector;
	float TargetSplineDistance = 0.0f;
	float CurrentSplineDistance = 0.0f;
	int DistanceBetweenSplinePoint;
	FVector SplineLocationPoint;
	FVector SplineTangent;

	bool GoToEndOfSpline;
	bool HasNewSplineBeenSetup = false;

	//rotator for turret
	FRotator StartingRotation;
	FRotator RotationIncrement;
	FRotator TargetRotation;
	FRotator NewRotation;
	float InterpSpeed = 1;

	//enemy
	FVector EnemyLocation;

	//shoot
	class AMinigun* Minigun = nullptr;
	class AHomingMissileLauncher* HomingMissileLauncher = nullptr;

	bool Overheating = false;

	//missile 
	bool MinigunPulledTrigger = false;
	bool MissileIsAvailable = false;
	int32 MissileChargeAmount = FMath::RandRange(1, 3);
	
	bool HasKilled = false;

	//timer
	bool TimerIsActive = false;
	bool TimerFirstTime = true;

	//Sensors
	float SensorGapDifference;
	USceneComponent* LeftSensor;
	USceneComponent* RightSensor;

	//timers - borde kollas igenom om dessa ska användas
	int TimeElapsed;
	int TurretDelayTime = FMath::RandRange(1.0f, 3.0f);

	FTimerHandle TimerHandle_SetStartingRotation;
	FTimerHandle TimerHandle_ResetRotationFlag;

	//Functions
	//driving behavior functions
	void DrivePath();
	void DriveAndShoot();

	//turret rotation
	void SetStartingRotation();
	void AddNewTurretRotation();

	//shoot
	void Shoot();

	//helper function
	bool InitializeSensors();
	bool InitializeSpline();
	
};
