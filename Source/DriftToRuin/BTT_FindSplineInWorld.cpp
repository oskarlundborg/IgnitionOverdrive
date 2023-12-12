// Fill out your copyright notice in the Description page of Project Settings.


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

	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}
	
	//göras om till en bool för att sen köra om ifall den inte hitta och lägga i progress
	if(!ScanForSplines())
	{
		UE_LOG(LogTemp, Warning, TEXT("spline finding in progress"));
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
}

bool UBTT_FindSplineInWorld::ScanForSplines() const
{
	// Get the player controller

	//scan is not that pretty, leave for now and work on AI find opponent instead. 

	// Parameters for the scan
	float ScanRadius = 1500.0f;
	float TraceDistance = 100;
	float Offset = 1400;
	// Calculate the start and end points for the scan
	//FVector ForwardVector = AIPawn->GetActorForwardVector();
	//FVector ScanStart = AIPawn->GetActorLocation();

	FVector ForwardVector = AIPawn->GetActorForwardVector();
	FVector ScanStart = AIPawn->GetActorLocation() + ForwardVector * Offset; // Adjust Offset as needed
	FVector ScanEnd = ScanStart + ForwardVector * TraceDistance; // Adjust TraceDistance as needed

	// Perform the scan
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(AIPawn);

	DrawDebugSphere(GetWorld(), ScanStart, ScanRadius, 12, FColor::Green, false, 15.f); // Visualize the starting sphere
	//DrawDebugLine(GetWorld(), ScanStart, ScanEnd, FColor::Green, false, 15.0f);
	//get current spline in blackboard
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("TempRoadSpline"));
	USplineComponent* BBSpline = nullptr;
	if (ActorRoadSpline != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACTOR ROAD SPLINE WAS NOT NULL SETTING BB SPLINE: "));
		BBSpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	}
	
	FVector PointToDriveTo = BlackboardComp->GetValueAsVector("LocationToDrive");
	TArray<int32> Differences;
// set spec trace channel for spline
	if (AIPawn->GetWorld()->SweepMultiByChannel(HitResults, ScanStart, ScanEnd, FQuat::Identity, ECC_Visibility,
	                                            FCollisionShape::MakeSphere(ScanRadius), CollisionParams))
	{
		// Array to store eligible spline hits
		TArray<USplineComponent*> EligibleSplineHits;
		//UE_LOG(LogTemp, Warning, TEXT("searching for splines"));

		// Filter hits to only include spline components with starting points in front of the AI
		for (const FHitResult& HitResult : HitResults)
		{
		
			USplineComponent* SplineComponent = Cast<USplineComponent>(
				HitResult.GetActor()->GetComponentByClass<USplineComponent>());

			if (SplineComponent && (SplineComponent != BBSpline || HitResult.GetActor() != ActorRoadSpline))
			{
				if(BBSpline)
				{
					UE_LOG(LogTemp, Error, TEXT("BBSPLINE: %s"), *BBSpline->GetName());
				}
				UE_LOG(LogTemp, Error, TEXT("splineComponent: %s"), *SplineComponent->GetName());
				UE_LOG(LogTemp, Error, TEXT("actor being added as spline hit result: %s"), *HitResult.GetActor()->GetName());
				EligibleSplineHits.Add(SplineComponent);
			}
		}
		// Choose a random spline from the eligible hits
		//make sure spline is not the one currently set
		if (EligibleSplineHits.Num() > 0)
		{
			TArray<float> Differences;

			//hitta en spline som är närmaste till mitt punkten, för att tvinga in Ai i mitten för mer fights. 
			for (USplineComponent* Spline : EligibleSplineHits)
			{
				FVector SplineMidpoint = Spline->GetLocationAtSplineInputKey(Spline->GetNumberOfSplinePoints() / 2, ESplineCoordinateSpace::World);
				float Distance = FVector::DistSquared(SplineMidpoint, PointToDriveTo);
				Differences.Add(Distance);
			}
			
			int32 MinDistanceIndex = 0;
			for (int32 i = 1; i < Differences.Num(); ++i)
			{
				if (Differences[i] < Differences[MinDistanceIndex])
				{
					MinDistanceIndex = i;
				}
			}

			float RandomChance = FMath::FRand();
			USplineComponent* ClosestSpline = EligibleSplineHits[MinDistanceIndex];
			
			//UE_LOG(LogTemp, Warning, TEXT("Spline was found"));
			USplineComponent* ChosenSpline = EligibleSplineHits[FMath::RandRange(0, EligibleSplineHits.Num() - 1)];
			// Set chosen spline in BB
			BlackboardComp->SetValueAsObject("RoadSpline", RandomChance <= 0.7f ? ClosestSpline->GetOwner() : ChosenSpline->GetOwner());
			AEnemyVehiclePawn* AIEnemy = Cast<AEnemyVehiclePawn>(AIPawn);
			ensureMsgf(AIEnemy != nullptr, TEXT("AI enemy was nyll, need it to set that new spline should be setup"));
			AIEnemy->SetHasNewSplineBeenSetup(false);
			return true;
		}
	}
	return false;
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
