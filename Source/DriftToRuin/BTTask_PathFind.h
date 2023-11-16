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
	
private:
	APawn* AIPawn;
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	FVector Destination;
};
