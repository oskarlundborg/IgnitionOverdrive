//Daniel Olsson - AI letar efter splines. 

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindSplineInWorld.generated.h"

class USplineComponent;

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_FindSplineInWorld : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_FindSplineInWorld();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

private:
	//AI properties
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	APawn* AIPawn;
	UPROPERTY()
	UBlackboardComponent* BlackboardComp;

	// Parameters for the scanRadius
	UPROPERTY(EditAnywhere)
	float ScanRadius = 1000.0f;
	UPROPERTY(EditAnywhere)
	float TraceDistance = 100.0f;
	UPROPERTY(EditAnywhere)
	float Offset = 900.0f;

	// Scans for nearby splines and returns true if any are found.
	bool ScanForSplines() const;

	// Finds hit results for splines in the game world and populates the HitResults array.
	void FindSplineHitResults(TArray<FHitResult>& HitResults, USplineComponent* BBSpline, TArray<USplineComponent*>& EligibleSplineHits) const;

	// Chooses the most suitable spline from the given spline hits.
	void ChooseAdequiteSpline(TArray<USplineComponent*>& SplineHits, TArray<USplineComponent*>& SplinesInFront) const;

	// Determines if a spline can be set based on certain conditions and returns a boolean result.
	auto bCanSetSpline(TArray<USplineComponent*>&, bool& bValue, TArray<USplineComponent*>& SplinesInFront) const -> bool;

	// Initializes AI components, cancels the task if it fails to initialize
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
};
