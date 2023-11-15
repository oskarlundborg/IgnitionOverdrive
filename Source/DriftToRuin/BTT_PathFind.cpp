// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_PathFind.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTT_PathFind::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// Get references to necessary components
	AIController = OwnerComp.GetAIOwner();
	ensureMsgf(AIController != nullptr, TEXT("AI controller was nullptr"));
	if (AIController != nullptr)
	{
		AIPawn = AIController->GetPawn();
	}

	// Access the Blackboard
	BlackboardComp = OwnerComp.GetBlackboardComponent();
	ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	if (BlackboardComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// Your destination point (replace this with your actual destination point)
	Destination = BlackboardComp->GetValueAsVector("PointLocation");;

	ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
	if(AIPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AIPawn->SetActorLocation(Destination);
	int r = 5;
	
	return EBTNodeResult::Succeeded;
}


