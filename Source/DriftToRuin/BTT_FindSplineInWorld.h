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
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	APawn* AIPawn;
	UPROPERTY()
	UBlackboardComponent* BlackboardComp;
	
	bool ScanForSplines() const;
	void FindSplineHitResults(TArray<FHitResult>& HitResults, USplineComponent* BBSpline, TArray<USplineComponent*>& EligibleSplineHits) const;
	void ChooseAdequiteSpline(TArray<USplineComponent*>& SplineHits, TArray<USplineComponent*>& SplinesInFront) const;
	auto bCanSetSpline(TArray<USplineComponent*>&, bool& bValue, TArray<USplineComponent*>& SplinesInFront) const -> bool;
	
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
};
