// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_PathFind.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"
#include "Components/SplineComponent.h"

UBTTask_PathFind::UBTTask_PathFind()
{
	NodeName = "BTT AICar Path finding";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PathFind::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!InitializeAIComponents(OwnerComp))
	{
		return EBTNodeResult::Failed;
	}

	if (!InitializeSplineAndSensors())
	{
		return EBTNodeResult::Failed;
	}

	VehicleMovementComponent = Cast<UChaosVehicleMovementComponent>(AIPawn->GetMovementComponent());
	ensureMsgf(VehicleMovementComponent != nullptr, TEXT("VehicleMovementComp was null"));
	if (VehicleMovementComponent == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Warning, TEXT("task is being run x times"));
	//	MoveToLocation(Destination);
	return EBTNodeResult::InProgress;
}

void UBTTask_PathFind::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	//UE_LOG(LogTemp, Warning, TEXT("tick task is being run x times"));

	//clamp a speed here.
	float Speed = VehicleMovementComponent->GetForwardSpeed();
	UE_LOG(LogTemp, Warning, TEXT("speed: %f"), Speed);
	if (Speed > 500)
	{
		VehicleMovementComponent->SetThrottleInput(VehicleMovementComponent->GetThrottleInput() - 0.2);
		UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0.4);
	}

	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	//make 500 into a check variable later

	float DistanceCheck = FVector::Dist(SplineLocationPoint, AIPawn->GetActorLocation());
	if (DistanceCheck < 500)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetSplineDistane before: %f"), TargetSplineDistance);
		TargetSplineDistance += 500;
		UE_LOG(LogTemp, Warning, TEXT("TargetSplineDistane after: %f"), TargetSplineDistance);
		UE_LOG(LogTemp, Warning, TEXT("distance CHEck: %f"), DistanceCheck);
	}


	//detta borde köras bara en gång per ny spline point, inte flera gånger. när man svänger, svänger man oftast med en och samma sväng kurva. 
	if (FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) > FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint))
	{
		//byter < här

		//vad sker när vänster senor på bil är närmare punkten 

		//beräkna vinkel till punkten? som ska på något sätt bestämma hur mycket jag svänger och vilken hastighet jag väljer

		VehicleMovementComponent->SetSteeringInput(0.6);

		//när ska bilen sluta svänga? Kolla hur du gör där nere i din första prototyp 
	}
	else
	{
		//vad sker när höger sensor på bil är närmare punkten

		//beräkna vinkel till punkten? som ska på något sätt bestämma hur mycket jag svänger och vilken hastighet jag väljer

		VehicleMovementComponent->SetSteeringInput(-0.6);

		//när ska bilen sluta svänga? Kolla hur du gör där nere i din första prototyp
	}


	/*//vinkel till kommande punkt
	SplineTangent = MySpline->GetTangentAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	if (AIPawn->GetActorRotation().Yaw < SplineTangent.Rotation().Yaw)
	{
		VehicleMovementComponent->SetSteeringInput(0.3);
	}
	else
	{
		VehicleMovementComponent->SetSteeringInput(-0.3);
	}


	// Calculate the direction from the car to the destination
	FVector DirectionToDestination = (Destination - AIPawn->GetActorLocation()).GetSafeNormal();

	// Calculate the forward vector of the car
	FVector ForwardVector = AIPawn->GetActorForwardVector().GetSafeNormal();

	// Calculate the dot product to check if the car is facing roughly in the direction of the destination
	float DotProduct = FVector::DotProduct(DirectionToDestination, ForwardVector);

	// Use a threshold to determine if the car is heading straight
	if
	(DotProduct
		>
		0.99f
	) // Adjust the threshold as needed
	{
		VehicleMovementComponent->SetSteeringInput(0);
		UE_LOG(LogTemp, Warning, TEXT("setting steer input to zero"));
	}*/

	if (FVector::Dist(AIPawn->GetActorLocation(), Destination) < DistanceThreshold)
	{
		VehicleMovementComponent->SetThrottleInput(0);
		VehicleMovementComponent->SetSteeringInput(0);
		UE_LOG(LogTemp, Warning, TEXT("setting throttle to zero and task has been succeeded"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

bool UBTTask_PathFind::InitializeAIComponents(UBehaviorTreeComponent& OwnerComp)
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

	return true;
}

bool UBTTask_PathFind::InitializeSplineAndSensors()
{
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("RoadSpline"));
	const AActor* CarActor = Cast<AActor>(BlackboardComp->GetValueAsObject("ObjectCar"));

	if (ActorRoadSpline == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spline Component not found on AIPawn., Actor roadspline was null"));
		return false;
	}

	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();

	TArray<UActorComponent*> LeftSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "LeftSensor");
	TArray<UActorComponent*> RightSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "RightSensor");
	LeftSensor = LeftSensors.Num() > 0 ? Cast<USceneComponent>(LeftSensors[0]) : nullptr;
	RightSensor = RightSensors.Num() > 0 ? Cast<USceneComponent>(RightSensors[0]) : nullptr;

	if (LeftSensor == nullptr || RightSensor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sensors were nullptr"));
		return false;
	}

	return true;
}
