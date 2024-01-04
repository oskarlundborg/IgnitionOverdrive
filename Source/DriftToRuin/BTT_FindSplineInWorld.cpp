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

	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}

	//starta om trädet ifall den inte hittar en spline. 
	if (!ScanForSplines())
	{
		OwnerComp.RestartTree();
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}


bool UBTT_FindSplineInWorld::ScanForSplines() const
{
	// Parameters for the scanraduis
	float ScanRadius = 1000.0f;
	float TraceDistance = 100.0f;
	float Offset = 1200.0f;

	//scan parameters
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(AIPawn);

	//Last previous spline
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("TempRoadSpline"));
	USplineComponent* BBSpline = nullptr;
	if (ActorRoadSpline != nullptr)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ACTOR ROAD SPLINE WAS NOT NULL SETTING BB SPLINE: "));
		BBSpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	}

	//vector values
	FVector ForwardVector = AIPawn->GetActorForwardVector();
	FVector ScanStart = AIPawn->GetActorLocation() + FVector(0.0f, 0.0f, 130.0f) + ForwardVector * Offset;
	FVector ScanEnd = ScanStart + ForwardVector * TraceDistance;

	// Perform the scan
	if (AIPawn->GetWorld()->SweepMultiByChannel(HitResults, ScanStart, ScanEnd,
	                                            AIPawn->GetActorLocation().Rotation().Quaternion(), ECC_Visibility,
	                                            FCollisionShape::MakeBox(
		                                            FVector(ScanRadius, ScanRadius, ScanRadius / 3)), CollisionParams))
	{
		// Array to store eligible spline hits
		TArray<USplineComponent*> EligibleSplineHits;
		// Filter hits to only include spline components with starting points in front of the AI
		FindSplineHitResults(HitResults, BBSpline, EligibleSplineHits);
		TArray<USplineComponent*> SplinesInFront;
		bool bValue;
		if (bCanSetSpline(EligibleSplineHits, bValue, SplinesInFront))
		{
			return true;
		}
	}
	return false;
}

void UBTT_FindSplineInWorld::FindSplineHitResults(TArray<FHitResult>& HitResults, USplineComponent* BBSpline,
                                                  TArray<USplineComponent*>& EligibleSplineHits) const
{
	for (const FHitResult& HitResult : HitResults)
	{
		USplineComponent* SplineComponent = Cast<USplineComponent>(
			HitResult.GetActor()->GetComponentByClass<USplineComponent>());

		if (SplineComponent && (SplineComponent != BBSpline /*|| HitResult.GetActor() != ActorRoadSpline*/))
		{
			EligibleSplineHits.Add(SplineComponent);
		}
	}
}


bool UBTT_FindSplineInWorld::bCanSetSpline(TArray<USplineComponent*>& EligibleSplineHits, bool& bValue,
                                           TArray<USplineComponent*>& SplinesInFront) const
{
	if (EligibleSplineHits.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("splines found"));
		//hitta splines framför bilen
		ChooseAdequiteSpline(EligibleSplineHits, SplinesInFront);
		USplineComponent* ChosenSpline;
		if (SplinesInFront.IsEmpty())
		{
			ChosenSpline = EligibleSplineHits[FMath::RandRange(0, EligibleSplineHits.Num() - 1)];
		}
		else
		{
			ChosenSpline = SplinesInFront[FMath::RandRange(0, SplinesInFront.Num() - 1)];
		}
		// Set chosen spline in BB
		BlackboardComp->SetValueAsObject("RoadSpline", ChosenSpline->GetOwner());
		AEnemyVehiclePawn* AIEnemy = Cast<AEnemyVehiclePawn>(AIPawn);
		if (AIEnemy != nullptr)
		{
			AIEnemy->SetHasNewSplineBeenSetup(false);
		}
		bValue = true;
		return true;
	}
	return false;
}

void UBTT_FindSplineInWorld::ChooseAdequiteSpline(TArray<USplineComponent*>& EligibleSplineHits,
                                                  TArray<USplineComponent*>& SplinesInFront) const
{
	if (EligibleSplineHits.IsEmpty())
	{
		return;
	}

	for (USplineComponent* Spline : EligibleSplineHits)
	{
		FVector SplineStart = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = Spline->GetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1,
		                                                     ESplineCoordinateSpace::World);

		FVector ActorToStart = SplineStart - AIPawn->GetActorLocation();
		FVector ActorToEnd = SplineEnd - AIPawn->GetActorLocation();

		// Check the dot product to determine if the spline is in front of the actor
		float DotStart = FVector::DotProduct(AIPawn->GetActorForwardVector(), ActorToStart.GetSafeNormal());
		float DotEnd = FVector::DotProduct(AIPawn->GetActorForwardVector(), ActorToEnd.GetSafeNormal());


		if ((DotStart >= 0.0f && DotEnd >= 0.0f))
		{
			SplinesInFront.Add(Spline);
		}
	}
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
