//Daniel Olsson - task som skapar en spline framåt längs bilens framåt rotation, änvändes inte i slutprodukt 
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "BTT_CreateSpline.generated.h"

/**
 *
This class was ultimately not utilized. Due to time constraints, we opted for manually placing splines in the world, as we were unable to implement and thoroughly test this solution.
 */
UCLASS()
class DRIFTTORUIN_API UBTT_CreateSpline : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_CreateSpline();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

	
	UPROPERTY(EditAnywhere)
	float MaxTraceDistance = 10000;
	
	UPROPERTY(EditAnywhere)
	float SplinePointThreshold = 200;

private:
	UPROPERTY()
	APawn* AIPawn;
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	UBlackboardComponent* BlackboardComp;
	UPROPERTY()
	FVector Destination;
	UPROPERTY()
	class UChaosVehicleMovementComponent* VehicleMovementComponent = nullptr;
	UPROPERTY()
	class USplineComponent* MySpline;

	//spline
	UPROPERTY()
	float TargetSplineDistance = 0.0f;
	UPROPERTY()
	float CurrentSplineDistance = 0.0f;
	UPROPERTY()
	int DistanceBetweenSplinePoint;
	UPROPERTY()
	FVector SplineLocationPoint;
	UPROPERTY()
	FVector SplineTangent;

	
	void CreateSpline();
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
	
};
