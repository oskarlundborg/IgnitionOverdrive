// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PathFind.generated.h"


/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTTask_PathFind : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PathFind();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere)
	float DistanceThreshold = 500;

	UPROPERTY(EditAnywhere)
	float MaxTraceDistance = 10000;
	UPROPERTY(EditAnywhere)
	float SplinePointThreshold = 200;

private:
	
	APawn* AIPawn;
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	FVector Destination;
	class UChaosVehicleMovementComponent* VehicleMovementComponent = nullptr;

	class USplineComponent* MySpline;

	//spline
	float TargetSplineDistance = 0.0f;
	float CurrentSplineDistance = 0.0f;
	int DistanceBetweenSplinePoint;
	FVector SplineLocationPoint;
	FVector SplineTangent;

	//sensor point
	USceneComponent* LeftSensor;
	USceneComponent* RightSensor;

	//helper function
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
	bool InitializeSplineAndSensors();
};

