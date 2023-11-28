// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.generated.h"

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

	
	//pathfinding
	UPROPERTY(EditAnywhere)
	float SplineEndPointDistanceThreshold = 1000;
	UPROPERTY(EditAnywhere)
	float NextPointOnSplineThreshold = 1000;

	// car driving
	UPROPERTY(EditAnywhere)
	float ThrottleInput;
	UPROPERTY(EditAnywhere)
	float BrakeInput;
	UPROPERTY(EditAnywhere)
	float SteeringInput;

private:
	
	FString SwitchString = "Drive";


	//Common Components
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	class USplineComponent* MySpline;
	class UChaosVehicleMovementComponent* VehicleMovementComponent = nullptr;

	
	//spline values
	FVector Destination = FVector::ZeroVector;
	float TargetSplineDistance = 0.0f;
	float CurrentSplineDistance = 0.0f;
	int DistanceBetweenSplinePoint;
	FVector SplineLocationPoint;
	FVector SplineTangent;

	//rotator for turret
	FRotator StartingRotation;
	FRotator RotationIncrement;
	FRotator TargetRotation;
	FRotator NewRotation;
	float InterpSpeed = 1;

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

	//helper function
	bool InitializeSplineAndSensors();
	
};
