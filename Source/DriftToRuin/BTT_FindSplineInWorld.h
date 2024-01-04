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
	AAIController* AIController;
	APawn* AIPawn;
	UBlackboardComponent* BlackboardComp;
	
	bool ScanForSplines() const;
	void FindSplineHitResults(TArray<FHitResult>& HitResults, USplineComponent* BBSpline, TArray<USplineComponent*>& EligibleSplineHits) const;
	void ChooseAdequiteSpline(TArray<USplineComponent*>&) const;
	auto bCanSetSpline(TArray<USplineComponent*>&, bool& bValue) const -> bool;
	
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
};
