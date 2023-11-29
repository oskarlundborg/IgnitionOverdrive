// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyVehiclePawn.h"

#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "FrameTypes.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"
#include "Perception/AIPerceptionComponent.h"

AEnemyVehiclePawn::AEnemyVehiclePawn()
{
}

void AEnemyVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f);

	//GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetStartingRotation, this,
	//                                      &AEnemyVehiclePawn::SetStartingRotation, 0.5f, false);

	if (PlayerTurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;
	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::KeepRelativeTransform,
	                           TEXT("Root_Turret"));
	Minigun->SetOwner(this);

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	                                  TEXT("Root_MissileLauncher"));
	HomingLauncher->SetOwner(this);

	// set turret starting location check if this works or i need the timer 
	SetStartingRotation();

	//get common components
	AIController = Cast<AAIController>(GetController());
	if (AIController != nullptr)
	{
		BlackboardComp = AIController->GetBlackboardComponent();
		ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	}

	VehicleMovementComponent = Cast<UChaosVehicleMovementComponent>(GetMovementComponent());
	ensureMsgf(VehicleMovementComponent != nullptr, TEXT("Vehicle movement comp was null"));
	if (VehicleMovementComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement cojmponent null"));
	}
}

void AEnemyVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//välj körnings beteende
	if (SwitchString == "DriveAndShoot")
	{
		DriveAndShoot();
		// Code for Case2
	}
	else if (SwitchString == "Drive")
	{
		InitializeSplineAndSensors();
		DrivePath();
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


void AEnemyVehiclePawn::DrivePath()
{
	RandomlyRotateTurret();

	ManageSpeed();

	DriveAlongSpline();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::DriveAndShoot()
{
	UE_LOG(LogTemp, Warning, TEXT("driving and shooting"));
	ManageSpeed();

	DriveAlongSpline();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::RandomlyRotateTurret()
{
	//rotera x antal vinklar efter en viss delay
	TimeElapsed = GetWorld()->GetTimeSeconds();
	if (!TimerIsActive)
	{
		TimerIsActive = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetRotationFlag, this,
		                                       &AEnemyVehiclePawn::AddNewTurretRotation,
		                                       TimerFirstTime ? 0.6f : TurretDelayTime, false);
		if (TimerFirstTime) TimerFirstTime = false;
	}
	//smooth rotation
	NewRotation = FMath::RInterpTo(GetTurret()->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), InterpSpeed);
	GetTurret()->SetActorRotation(NewRotation);
}

void AEnemyVehiclePawn::ManageSpeed()
{
	const float Speed = VehicleMovementComponent->GetForwardSpeed();
	if (Speed > 800)
	{
		VehicleMovementComponent->SetThrottleInput(VehicleMovementComponent->GetThrottleInput() - 0.2);
		//		UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0.4);
	}
}

void AEnemyVehiclePawn::AddNewTurretRotation()
{
	//add a new rotation to the target rotation
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f);
	RotationIncrement = FMath::RandBool() ? FRotator(0, 50, 0) : FRotator(0, -50, 0);
	StartingRotation = GetTurret()->GetActorRotation();
	TargetRotation.Yaw = StartingRotation.Yaw + RotationIncrement.Yaw;
	TimerIsActive = false;
}

void AEnemyVehiclePawn::DriveAlongSpline()
{
	if (LeftSensor == nullptr || RightSensor == nullptr || VehicleMovementComponent == nullptr || MySpline == nullptr)
		return;
	//get a spline point along the spline
	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	//check if car has reached point, then get a new point
	float DistanceCheck = FVector::Dist(SplineLocationPoint, GetActorLocation());
	if (DistanceCheck < NextPointOnSplineThreshold)
	{
		TargetSplineDistance += 500;
	}
	//check which sensor is closer to point
	SensorGapDifference = FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) - FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint);
	SensorGapDifference = FMath::Abs(SensorGapDifference);

	float TempSteeringInput = SteeringInput;
	if (SensorGapDifference < 10)
	{
		SteeringInput = 0;
	}
	else if (FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) > FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint))
	{
		SteeringInput = 0.6;
	}
	else
	{
		SteeringInput = -0.6;
	}
	//lerp the steering input for smoother turn
	// find out waht delta time * 40 does
	const float LerpValue = FMath::Lerp(TempSteeringInput, SteeringInput, GetWorld()->DeltaTimeSeconds * 40);
	//lerp for smoother turning curve
	TempSteeringInput = LerpValue;
	VehicleMovementComponent->SetSteeringInput(LerpValue);
}

void AEnemyVehiclePawn::CheckIfAtEndOfSpline()
{
	if (BlackboardComp != nullptr)
	{
		Destination = BlackboardComp->GetValueAsVector("PointLocation");
	}
	if (FVector::Dist(GetActorLocation(), Destination) < SplineEndPointDistanceThreshold)
	{
		VehicleMovementComponent->SetThrottleInput(0);
		VehicleMovementComponent->SetSteeringInput(0);
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
	if (MySpline == nullptr) return false;

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

void AEnemyVehiclePawn::SetStartingRotation()
{
	StartingRotation = GetTurret()->GetActorRotation();
}

void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}
