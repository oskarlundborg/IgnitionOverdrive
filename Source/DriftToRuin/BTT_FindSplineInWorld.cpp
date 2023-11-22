// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindSplineInWorld.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"

UBTT_FindSplineInWorld::UBTT_FindSplineInWorld()
{
}

EBTNodeResult::Type UBTT_FindSplineInWorld::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}
	ScanForSplines();

	return EBTNodeResult::Succeeded;
}

void UBTT_FindSplineInWorld::ScanForSplines() const
{
	// Get the player controller

	//scan is not that pretty, leave for now and work on AI find opponent instead. 

	
	// Parameters for the scan
	float ScanRadius = 2000.0f;
	float ScanAngle = 90.0f;
	float TraceDistance = 100;
	float Offset = 10;
	// Calculate the start and end points for the scan
	//FVector ForwardVector = AIPawn->GetActorForwardVector();
	//FVector ScanStart = AIPawn->GetActorLocation();

	FVector ForwardVector = AIPawn->GetActorForwardVector();
	FVector ScanStart = AIPawn->GetActorLocation() + ForwardVector * Offset;  // Adjust Offset as needed
	FVector ScanEnd = ScanStart + ForwardVector * TraceDistance;  // Adjust TraceDistance as needed

	
	//this might be wierd
	//FVector ScanEnd = ScanStart + ForwardVector.RotateAngleAxis(ScanAngle * 0.5f, FVector::UpVector) * ScanRadius *
		2.0f;
	float ScanHalfHeight  = 100.0f;

	// Perform the scan
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(AIPawn);

	if (AIPawn->GetWorld()->SweepMultiByChannel(HitResults, ScanStart, ScanEnd, FQuat::Identity, ECC_Visibility,
	                                            FCollisionShape::MakeSphere(ScanRadius), CollisionParams))
	{
		// Array to store eligible spline hits
		TArray<USplineComponent*> EligibleSplineHits;
		UE_LOG(LogTemp, Warning, TEXT("searching for splines"));

		// Filter hits to only include spline components with starting points in front of the AI
		for (const FHitResult& HitResult : HitResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("in for loop of the hit results, result: %s"), *HitResult.ToString());
			if (USplineComponent* SplineComponent = Cast<USplineComponent>(HitResult.GetActor()->GetComponentByClass<USplineComponent>())) //denna är fel
			{
				UE_LOG(LogTemp, Warning, TEXT("actor spline hit "));
				FVector SplineStart = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
				FVector ToSplineStart = SplineStart - AIPawn->GetActorLocation();
				float DotProduct = FVector::DotProduct(ToSplineStart.GetSafeNormal(), ForwardVector);

				// Check if the spline starting point is in front of the AI
				if (DotProduct > 0.0f)
				{
					UE_LOG(LogTemp, Warning, TEXT("aadding spline"));
					EligibleSplineHits.Add(SplineComponent);
				}
			}
		}

		// Choose a random spline from the eligible hits
		if (EligibleSplineHits.Num() > 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spline was found"));
			USplineComponent* ChosenSpline = EligibleSplineHits[FMath::RandRange(0, EligibleSplineHits.Num() - 1)];
			// Set chosen spline in BB
			BlackboardComp->SetValueAsObject("RoadSpline", ChosenSpline->GetOwner());
			
		}
	}

	// Draw debug sphere for visualization (optional)
	//DrawDebugSphere(AIPawn->GetWorld(), ScanStart, ScanRadius, 32, FColor::Green, false, 20.0f);
	DrawDebugCapsule(AIPawn->GetWorld(), ScanStart, ScanHalfHeight, ScanRadius, FQuat::Identity, FColor::Green, false, 10.0f);


}


bool UBTT_FindSplineInWorld::InitializeAIComponents(UBehaviorTreeComponent& OwnerComp)
{
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
		return false;
	}

	ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
	if (AIPawn == nullptr)
	{
		return false;
	}

	return true;
}
