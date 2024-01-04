//Daniel Olsson


#include "BTT_CreateSpline.h"
#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"


UBTT_CreateSpline::UBTT_CreateSpline()
{
	
}

EBTNodeResult::Type UBTT_CreateSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}
	
	CreateSpline();

	return EBTNodeResult::Succeeded;
}

//göra om detta till A* pathfinding spline creating. ! 

//göra om så att den tar en annan spline, ka nej ta custom defined spline och göra om de


void UBTT_CreateSpline::CreateSpline()
{
	const FVector CurrentLocation = AIPawn->GetActorLocation();

	if (MySpline != nullptr)
	{
		MySpline->ClearSplinePoints();

		//måste göras till en loop, så att spline points med mer än två points fungerar
		
		//få spline length
		float SplineDistance = FMath::Abs(FVector::Dist(CurrentLocation, Destination));
		
		// distans mellan varje punkt? 1000 units?

		//bestäm antal punkter på spline
		int TotalSplinePoints = FMath::DivideAndRoundNearest(SplineDistance, 1000.0f);

		//beräkna distans mellan varje punkt för att sätta ut punkter jämnt på splinen. Avrundning till int för att skippa float dumheter
		DistanceBetweenSplinePoint = static_cast<int>(SplineDistance) / TotalSplinePoints;
		
		//trace up aswell innan trace ner, så vi inte kommer förbi ett objekt ovanifrån. 
		
		// Trace down from CurrentLocation to find the ground and set the first spline point
		FVector TraceStart = CurrentLocation + FVector(0, 0, 5000);
		FVector TraceEnd = TraceStart - FVector::UpVector * MaxTraceDistance; // Adjust MaxTraceDistance as needed
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(AIPawn); // Ignore the car itself

		for (int i = 0; i < TotalSplinePoints; i++)
		{
			if (i == 0)
			{
				if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility,
				                                         CollisionParams))
				{
					MySpline->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), HitResult.Location, 25.0f, 8, FColor::Red, false, 20.0f);
				}
			}
			else
			{
				// kollas igenom, tror denna beräkning ör fel. ibland sätts sista punkten lite skumt. 
				TraceStart = CurrentLocation + (Destination - CurrentLocation).GetSafeNormal()/*.GetAbs() */ *
					DistanceBetweenSplinePoint * i + FVector(0, 0, 5000);
				TraceEnd = TraceStart - FVector::UpVector * MaxTraceDistance;
				if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility,
				                                         CollisionParams))
				{
					MySpline->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), HitResult.Location, 25.0f, 8, FColor::Red, false, 20.0f);
				}
			}
		}

		// Trace down from DestinationPoint to find the ground and set the second spline point
		TraceStart = Destination + FVector(0, 0, 5000);
		TraceEnd = TraceStart - FVector::UpVector * MaxTraceDistance; // Adjust MaxTraceDistance as needed

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
		{
			MySpline->AddSplinePoint(HitResult.Location, ESplineCoordinateSpace::World);
		}

		//will need loop if more spline locations

		//find spline location point
		// Use AddInputVector to apply movement input in the calculated direction
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline Component not found on AIPawn."));
	}
}

bool UBTT_CreateSpline::InitializeAIComponents(UBehaviorTreeComponent& OwnerComp)
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

	// Your destination point (replace this with your actual destination point)
	Destination = BlackboardComp->GetValueAsVector("PointLocation");

	ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
	if (AIPawn == nullptr)
	{
		return false;
	}

	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("AIOwnedRoadSpline"));
	if (ActorRoadSpline == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline Component not found on AIPawn., Actor roadspline was null"));
		return false;
	}

	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();

	return true;
}
