// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_CarRoamAround.h"

#include "AIController.h"

EBTNodeResult::Type UBTT_CarRoamAround::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	MeterRange.Add(LowestMeterDistance);
	MeterRange.Add(HighestMeterDistance);

	const AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());

	ensureMsgf(AIController != nullptr, TEXT("AI controller was nullptr"));

	APawn* AIPawn = nullptr;
	if (AIController != nullptr)
	{
		AIPawn = AIController->GetPawn();
		ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
		if (AIPawn != nullptr)
		{
			//get camera
		}
	}

	// get a point within meter range and make sure its correct viewing degree (should not get a point behind car / camera) 
	if (MoveToPoint(PointLocation, AIPawn))
	{
		// Return Succeeded if the task is complete.
		return EBTNodeResult::Succeeded;
	}
	else
	{
		// Return InProgress if the task is still in progress.
		return EBTNodeResult::InProgress;
	}
}

bool UBTT_CarRoamAround::MoveToPoint(FVector& Point, APawn*)
{
	//implement A* pathfinding, and move to the point

	//a pawn could be null, if controller cant be found !!!!

	/*
	if ( is at point location )
	{
		// Return Succeeded if the task is complete.
		return true;
	}
	else
	{
		// Return InProgress if the task is still in progress.
		return EBTNodeResult::InProgress;
		moveToPoint(Point);
		return false; 
	}
	*/
	return false;
}


// this can be split up into different tasks ?

// one task for getting the point

// one task for moving to the point

