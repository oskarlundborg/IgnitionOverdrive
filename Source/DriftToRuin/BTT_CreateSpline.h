// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BTT_CreateSpline.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_CreateSpline : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_CreateSpline();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

	UPROPERTY(EditAnywhere)
	float DistanceThreshold = 1000;

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

	
	void CreateSpline();
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
	
};
