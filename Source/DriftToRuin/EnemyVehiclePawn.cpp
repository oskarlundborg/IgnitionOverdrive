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
#include "Kismet/KismetMathLibrary.h"
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


	DriveAlongSpline();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::DriveAndShoot()
{
	UE_LOG(LogTemp, Warning, TEXT("driving and shooting"));

	DriveAlongSpline();

	ManageSpeed();

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
	UE_LOG(LogTemp, Warning, TEXT("maxspeed before calculation: %f "), MaxSpeed);
	//kolla rotationen till nästa spline point, adjust speed beroende på storleken av vinkel (större sväng ger större vinkel, Yaw specifikt) 
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
	UE_LOG(LogTemp, Warning, TEXT("delta rotator %s"), *DeltaRotator.ToString());
	float DeltaYaw = FMath::Abs(DeltaRotator.Yaw);

	//5000 värdet kan lekas runt med, mindre värde ger mindre speed, högre värde ger mer speed. 
	DeltaYaw = 5000 / DeltaYaw;

	//slow down faster if max speed is bigger than delta yaw
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * MaxSpeed > DeltaYaw ? 20.0 : 0.1;

	MaxSpeed = FMath::Lerp(MaxSpeed, DeltaYaw, DeltaTime);
	UE_LOG(LogTemp, Warning, TEXT("maxspeed %f"), MaxSpeed);

	//clamp value betwwen low speed and maxspeed
	MaxSpeed = FMath::Clamp(MaxSpeed, 50, 3000);
	UE_LOG(LogTemp, Warning, TEXT("maxspeed after clamp %f"), MaxSpeed);
	//lerp
	//MaxSpeed
	//tempSpeed


	const float Speed = VehicleMovementComponent->GetForwardSpeed();
	if (Speed > MaxSpeed)
	{
		// be able to slow down very much faster in a curve
		VehicleMovementComponent->SetThrottleInput(VehicleMovementComponent->GetThrottleInput() - 0.2);
		UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0.6);
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
	//steering
	FVector CurrentVelocity = GetVelocity();
	FVector TempLocation = GetActorLocation();

	float DistBetweenPoint = FVector::Dist(TempLocation, SplineLocationPoint);
	float Time = DistBetweenPoint / CurrentVelocity.Length();
	float TimeStep = Time / 5;


	//check which sensor is closer to point
	SensorGapDifference = FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) - FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint);
	SensorGapDifference = FMath::Abs(SensorGapDifference);


	//steering
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
