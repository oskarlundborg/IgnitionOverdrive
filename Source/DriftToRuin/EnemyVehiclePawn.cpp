// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyVehiclePawn.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"

AEnemyVehiclePawn::AEnemyVehiclePawn()
{
}

void AEnemyVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretSocket"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	                           TEXT("MinigunSocket"));
	Minigun->SetOwner(this);

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	                                  TEXT("HomingSocket"));
	HomingLauncher->SetOwner(this);


	//get blackboard comp 
	AIController = Cast<AAIController>(GetController());
	if (AIController != nullptr)
	{
		BlackboardComp = AIController->GetBlackboardComponent();
		ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	}
	VehicleMovementComponent = Cast<UChaosVehicleMovementComponent>(GetMovementComponent());
	if (VehicleMovementComponent == nullptr)
	{
		//		UE_LOG(LogTemp, Warning, TEXT("movement cojmponent null"));
	}
}

void AEnemyVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	if (SwitchString == "Drive")
	{
		InitializeSplineAndSensors();
		DrivePath();
	}
	else if (SwitchString == "DriveAndShoot")
	{
		// Code for Case2
	}
	else if (SwitchString == "Rush")
	{
		// Code for Case3
	}
	else
	{
		// Default case
	}
}


void AEnemyVehiclePawn::SetSwitchString(const std::string& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

void AEnemyVehiclePawn::DrivePath()
{
	const float Speed = VehicleMovementComponent->GetForwardSpeed();
	//	UE_LOG(LogTemp, Warning, TEXT("speed: %f"), Speed);
	if (Speed > 800)
	{
		VehicleMovementComponent->SetThrottleInput(VehicleMovementComponent->GetThrottleInput() - 0.2);
		//		UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0.4);
	}

	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	//make 500 into a check variable later

	float DistanceCheck = FVector::Dist(SplineLocationPoint, GetActorLocation());
	if (DistanceCheck < 1000)
	{
		//	UE_LOG(LogTemp, Warning, TEXT("TargetSplineDistane before: %f"), TargetSplineDistance);
		TargetSplineDistance += 500;
		//UE_LOG(LogTemp, Warning, TEXT("TargetSplineDistane after: %f"), TargetSplineDistance);
		//	UE_LOG(LogTemp, Warning, TEXT("distance CHEck: %f"), DistanceCheck);
	}
	SensorGapDifference = FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) - FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint);

	SensorGapDifference = FMath::Abs(SensorGapDifference);
	UE_LOG(LogTemp, Warning, TEXT("sensor gap diiff : %f"), SensorGapDifference);
	if (SensorGapDifference < 10)
	{
		VehicleMovementComponent->SetSteeringInput(0);
		UE_LOG(LogTemp, Warning, TEXT("sensor gap diiff : %f"), SensorGapDifference);
	}
	else if (FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) > FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint))
	{
		VehicleMovementComponent->SetSteeringInput(0.6);
	}
	else
	{
		VehicleMovementComponent->SetSteeringInput(-0.6);
	}
	//	UE_LOG(LogTemp, Warning, TEXT("Destination, %f, %f, %f"), Destination.X, Destination.Y, Destination.Z);
	//	UE_LOG(LogTemp, Warning, TEXT("actor location, %s"), *GetActorLocation().ToString());
	if (BlackboardComp != nullptr)
	{
		Destination = BlackboardComp->GetValueAsVector("PointLocation");
	}

	if (FVector::Dist(GetActorLocation(), Destination) < DistanceThreshold)
	{
		VehicleMovementComponent->SetThrottleInput(0);
		VehicleMovementComponent->SetSteeringInput(0);
		//UE_LOG(LogTemp, Warning, TEXT("setting throttle to zero and task has been succeeded"));
		BlackboardComp->SetValueAsBool("AtRoadEnd", true);
	}
}


bool AEnemyVehiclePawn::InitializeSplineAndSensors()
{
	// kan säkert förenklas denna metod 
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("RoadSpline"));
	const AActor* CarActor = Cast<AActor>(BlackboardComp->GetValueAsObject("ObjectCar"));

	if (ActorRoadSpline == nullptr)
	{
		//	UE_LOG(LogTemp, Warning, TEXT("Spline Component not found on AIPawn., Actor roadspline was null"));
		return false;
	}

	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();

	TArray<UActorComponent*> LeftSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "LeftSensor");
	TArray<UActorComponent*> RightSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "RightSensor");
	LeftSensor = LeftSensors.Num() > 0 ? Cast<USceneComponent>(LeftSensors[0]) : nullptr;
	RightSensor = RightSensors.Num() > 0 ? Cast<USceneComponent>(RightSensors[0]) : nullptr;

	if (LeftSensor == nullptr || RightSensor == nullptr)
	{
		//	UE_LOG(LogTemp, Warning, TEXT("Sensors were nullptr"));
		return false;
	}

	return true;
}
