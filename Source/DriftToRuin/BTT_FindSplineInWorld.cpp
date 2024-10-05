// Daniel Olsson

#include "BTT_FindSplineInWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "EnemyVehiclePawn.h"
#include "Components/SplineComponent.h"

UBTT_FindSplineInWorld::UBTT_FindSplineInWorld()
{
}

EBTNodeResult::Type UBTT_FindSplineInWorld::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// Attempts to initialize AI components. If initialization fails, return failure status.
	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}

	// Scans for nearby splines. If no splines are found, restarts the behavior tree and returns failure status.
	if (!ScanForSplines())
	{
		OwnerComp.RestartTree();
		return EBTNodeResult::Failed;
	}

	// If all checks pass, return success status for the task.
	return EBTNodeResult::Succeeded;
}

// Scans the environment for splines within a specified radius and trace distance.
// Returns true if eligible splines are found; otherwise, returns false.

bool UBTT_FindSplineInWorld::ScanForSplines() const
{
	// Initialize an array to store hit results from the scan, and set collision parameters to ignore the AI pawn.
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(AIPawn);

	// Get the last known road spline actor from the blackboard and cast it to an AActor.
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("TempRoadSpline"));
	USplineComponent* BBSpline = nullptr;
	if (ActorRoadSpline != nullptr)
	{
		// If the road spline actor exists, retrieve its spline component.
		BBSpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	}

	// Set the start and end locations for the spline scan based on the AI pawn's forward vector and location.
	FVector ForwardVector = AIPawn->GetActorForwardVector();
	FVector ScanStart = AIPawn->GetActorLocation() + FVector(0.0f, 0.0f, 130.0f) + ForwardVector * Offset;
	FVector ScanEnd = ScanStart + ForwardVector * TraceDistance;
	
	// Perform a box-shaped sweep in the world to detect splines in the given range.
	if (AIPawn->GetWorld()->SweepMultiByChannel(HitResults, ScanStart, ScanEnd,
	                                            AIPawn->GetActorLocation().Rotation().Quaternion(), ECC_Visibility,
	                                            FCollisionShape::MakeBox(
		                                            FVector(ScanRadius, ScanRadius, ScanRadius / 2)), CollisionParams))
	{
		// Initialize an array to store the eligible spline hits.
		TArray<USplineComponent*> EligibleSplineHits;

		// Filter the hit results to only include spline components 
		FindSplineHitResults(HitResults, BBSpline, EligibleSplineHits);

		// Initialize an array for splines in front of the AI and a boolean to check if a spline can be set.
		TArray<USplineComponent*> SplinesInFront;
		bool bValue;

		// If an adequate spline can be set, return true, indicating the spline was found.
		if (bCanSetSpline(EligibleSplineHits, bValue, SplinesInFront))
		{
			return true;
		}
	}
	// Return false if no eligible splines were found.
	return false;
}

// Filters the hit results to find spline components, excluding the currently known spline from the blackboard.
// Adds eligible splines to the EligibleSplineHits array.

void UBTT_FindSplineInWorld::FindSplineHitResults(TArray<FHitResult>& HitResults, USplineComponent* BBSpline,
												  TArray<USplineComponent*>& EligibleSplineHits) const
{
	// Iterate through all hit results.
	for (const FHitResult& HitResult : HitResults)
	{
		// Attempt to retrieve a spline component from the actor hit by the scan.
		USplineComponent* SplineComponent = Cast<USplineComponent>(
			HitResult.GetActor()->GetComponentByClass<USplineComponent>());

		// If a spline component is found and it is not the one stored in the blackboard (BBSpline),
		// add it to the list of eligible spline hits.
		if (SplineComponent && SplineComponent != BBSpline )
		{
			EligibleSplineHits.Add(SplineComponent);
		}
	}
}

// Determines whether a spline can be set based on the eligible splines found in the world.
// Returns true if a spline is set, and updates the blackboard with the chosen spline.

bool UBTT_FindSplineInWorld::bCanSetSpline(TArray<USplineComponent*>& EligibleSplineHits, bool& bValue,
										   TArray<USplineComponent*>& SplinesInFront) const
{
	// Check if there are any eligible spline hits.
	if (EligibleSplineHits.Num() > 0)
	{
		// Choose splines that are in front of the AI vehicle.
		ChooseAdequiteSpline(EligibleSplineHits, SplinesInFront);

		USplineComponent* ChosenSpline;

		// If there are no splines in front, randomly choose from eligible splines.
		if (SplinesInFront.IsEmpty())
		{
			ChosenSpline = EligibleSplineHits[FMath::RandRange(0, EligibleSplineHits.Num() - 1)];
		}
		else
		{
			// Otherwise, randomly select one of the splines that are in front.
			ChosenSpline = SplinesInFront[FMath::RandRange(0, SplinesInFront.Num() - 1)];
		}

		// Set the chosen spline's owner in the blackboard as the new road spline object.
		BlackboardComp->SetValueAsObject("RoadSpline", ChosenSpline->GetOwner());

		// Check if the AI pawn is of type AEnemyVehiclePawn and set the spline setup flag to false if true.
		AEnemyVehiclePawn* AIEnemy = Cast<AEnemyVehiclePawn>(AIPawn);
		if (AIEnemy != nullptr)
		{
			AIEnemy->SetHasNewSplineBeenSetup(false);
		}

		// Indicate that a spline has been successfully set.
		bValue = true;
		return true;
	}

	// Return false if no eligible splines are found.
	return false;
}

// Filters eligible splines and identifies those that are in front of the AI actor based on their start and end points.
// Adds splines that meet this condition to the SplinesInFront array.

void UBTT_FindSplineInWorld::ChooseAdequiteSpline(TArray<USplineComponent*>& EligibleSplineHits,
												  TArray<USplineComponent*>& SplinesInFront) const
{
	// Return immediately if there are no eligible splines.
	if (EligibleSplineHits.IsEmpty())
	{
		return;
	}

	// Loop through each eligible spline.
	for (USplineComponent* Spline : EligibleSplineHits)
	{
		// Get the world location of the spline's start and end points.
		FVector SplineStart = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = Spline->GetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1,
															 ESplineCoordinateSpace::World);

		// Calculate the vector from the AI's location to the start and end points of the spline.
		FVector ActorToStart = SplineStart - AIPawn->GetActorLocation();
		FVector ActorToEnd = SplineEnd - AIPawn->GetActorLocation();

		// Use the dot product to check if both the start and end points of the spline are in front of the AI.
		float DotStart = FVector::DotProduct(AIPawn->GetActorForwardVector(), ActorToStart.GetSafeNormal());
		float DotEnd = FVector::DotProduct(AIPawn->GetActorForwardVector(), ActorToEnd.GetSafeNormal());

		// If both the start and end of the spline are in front of the AI, add the spline to SplinesInFront.
		if (DotStart >= 0.0f && DotEnd >= 0.0f)
		{
			SplinesInFront.Add(Spline);
		}
	}
}


// Initializes key AI components such as the AI controller, AI pawn, and blackboard.
// Ensures that these components are valid and returns false if any component is missing, canceling the process.

bool UBTT_FindSplineInWorld::InitializeAIComponents(UBehaviorTreeComponent& OwnerComp)
{
	// Get a reference to the AI controller from the behavior tree owner.
	AIController = OwnerComp.GetAIOwner();
    
	// Ensure the AI controller is valid; log a message if it's null.
	ensureMsgf(AIController != nullptr, TEXT("AI controller was nullptr"));
	if (AIController != nullptr)
	{
		// If the AI controller is valid, get the pawn it controls.
		AIPawn = AIController->GetPawn();
	}

	// Access the blackboard component from the behavior tree owner.
	BlackboardComp = OwnerComp.GetBlackboardComponent();
    
	// Ensure the blackboard component is valid; log a message if it's null.
	ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
    
	// If the blackboard is invalid, return false to indicate failure.
	if (BlackboardComp == nullptr)
	{
		return false;
	}

	// Ensure the AI pawn is valid; log a message if it's null.
	ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
    
	// If the AI pawn is null, return false to indicate failure.
	if (AIPawn == nullptr)
	{
		return false;
	}

	// If all components are valid, return true to indicate successful initialization.
	return true;
}
