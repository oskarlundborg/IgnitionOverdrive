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
	void SetSwitchString(const std::string& NewSwitchString);
	
	//pathfinding
	
	UPROPERTY(EditAnywhere)
	float DistanceThreshold = 1000;

	UPROPERTY(EditAnywhere)
	float MaxTraceDistance = 10000;
	UPROPERTY(EditAnywhere)
	float SplinePointThreshold = 200;
	
private:
	float ThrottleInput;
	float BrakeInput;
	float SteeringInput;

	std::string SwitchString = "Drive";


	//pathfinding
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	FVector Destination = FVector::ZeroVector;
	class UChaosVehicleMovementComponent* VehicleMovementComponent = nullptr;

	class USplineComponent* MySpline;

	//spline
	float TargetSplineDistance = 0.0f;
	float CurrentSplineDistance = 0.0f;
	int DistanceBetweenSplinePoint;
	FVector SplineLocationPoint;
	FVector SplineTangent;

	//difference in lenght between the two sensor towards the spline point  
	float SensorGapDifference;

	//sensor point
	USceneComponent* LeftSensor;
	USceneComponent* RightSensor;
	

	//helper function
	bool InitializeSplineAndSensors();
	
	void DrivePath();
	
};
