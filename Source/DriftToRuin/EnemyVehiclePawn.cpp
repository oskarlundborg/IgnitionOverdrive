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

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetStartingRotation, this,
	                                       &AEnemyVehiclePawn::SetStartingRotation, 0.5f, false);

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

	AIController = Cast<AAIController>(GetController());
	if (AIController != nullptr)
	{
		BlackboardComp = AIController->GetBlackboardComponent();
		ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	}
	
	VehicleMovementComponent = Cast<UChaosVehicleMovementComponent>(GetMovementComponent());
	if (VehicleMovementComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement cojmponent null"));
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
		DriveAndShoot();
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


void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

void AEnemyVehiclePawn::DrivePath()
{
//	UE_LOG(LogTemp, Warning, TEXT("timerisactive, %hhd"), TimerIsActive);
	TimeElapsed = GetWorld()->GetTimeSeconds();
	if (!TimerIsActive)
	{
		TimerIsActive = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetRotationFlag, this, &AEnemyVehiclePawn::RotateTurret,
		                                       TimerFirstTime ? 0.6f : TurretDelayTime, false);
		if (TimerFirstTime) TimerFirstTime = false;
	}

	//UE_LOG(LogTemp, Warning, TEXT("New Rotation before interp %s"), *NewRotation.ToString());
	NewRotation = FMath::RInterpTo(GetTurret()->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), InterpSpeed);

	//UE_LOG(LogTemp, Warning, TEXT("New Rotation after interp %s"), *NewRotation.ToString());


	//GetTurret()->AddActorWorldRotation(NewRotation);
	GetTurret()->SetActorRotation(NewRotation);

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
	float TempSteeringInput = SteeringInput;
	//UE_LOG(LogTemp, Warning, TEXT("sensor gap diiff : %f"), SensorGapDifference);
	if (SensorGapDifference < 10)
	{
		SteeringInput = 0;
		//UE_LOG(LogTemp, Warning, TEXT("sensor gap diiff : %f"), SensorGapDifference);
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
	// find out waht delta time * 40 does
	const float LerpValue = FMath::Lerp(TempSteeringInput, SteeringInput, GetWorld()->DeltaTimeSeconds * 40);
	//lerp for smoother turning curve
	TempSteeringInput = LerpValue;
	VehicleMovementComponent->SetSteeringInput(LerpValue);
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

void AEnemyVehiclePawn::DriveAndShoot()
{
	UE_LOG(LogTemp, Warning, TEXT("enemy detected"));
}

void AEnemyVehiclePawn::RotateTurret()
{
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f);
	RotationIncrement = FMath::RandBool() ? FRotator(0, 50, 0) : FRotator(0, -50, 0);
	//UE_LOG(LogTemp, Warning, TEXT("rotation increment %s"), *RotationIncrement.ToString());
	StartingRotation = GetTurret()->GetActorRotation();
//	UE_LOG(LogTemp, Warning, TEXT("starting rotaion  %s"), *StartingRotation.ToString());
	TargetRotation.Yaw = StartingRotation.Yaw + RotationIncrement.Yaw;
//	UE_LOG(LogTemp, Warning, TEXT("target rotation  %s"), *TargetRotation.ToString());
	TimerIsActive = false;
	//UE_LOG(LogTemp, Warning, TEXT("timer is active , %hhd"), TimerIsActive);
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

void AEnemyVehiclePawn::SetStartingRotation()
{
	StartingRotation = GetTurret()->GetActorRotation();
}
