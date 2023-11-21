// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindSplineInWorld.generated.h"

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
	void ScanForSplines() const;
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
};
