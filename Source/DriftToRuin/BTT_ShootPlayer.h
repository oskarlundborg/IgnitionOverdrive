// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ShootPlayer.generated.h"

class AMinigun;
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_ShootPlayer : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_ShootPlayer();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool HasKilled = false;
private:
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
	APawn* AIPawn;
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	AMinigun* Minigun;
};
