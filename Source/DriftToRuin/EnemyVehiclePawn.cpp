// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyVehiclePawn.h"
#include "AITurret.h"
#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyVehiclePawn::AEnemyVehiclePawn()
{
}

void AEnemyVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f);

	//GetWorld()->GetTimerManager().SetTimer(TimerHandle_SetStartingRotation, this,
	//                                      &AEnemyVehiclePawn::SetStartingRotation, 0.5f, false);

	if (AITurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;
	Turret = GetWorld()->SpawnActor<AAITurret>(AITurretClass);
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

	InitializeSensors();
	InitializeSpline();
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

	Shoot();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::Shoot()
{
	UE_LOG(LogTemp, Warning, TEXT("AI player shooting and rotating to turret, bullet now."));
	EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLocation);

	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), InterpSpeed);
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}
	ABaseVehiclePawn* Enemy = Cast<ABaseVehiclePawn>(BlackboardComp->GetValueAsObject("Enemy"));

	if (Enemy && Enemy->GetIsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("enemy is dead"));
		HasKilled = true;
		return;
	}

	TArray<AActor*> CarActors;
	GetAttachedActors(CarActors);
	for (AActor* ChildActor : CarActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("child actor AI TUREET: %s"), *ChildActor->GetName());
		// Check if the child actor is of type AMinigun
		if (Minigun == nullptr)
		{
			Minigun = Cast<AMinigun>(ChildActor);
		}
		if (HomingMissileLauncher == nullptr)
		{
			HomingMissileLauncher = Cast<AHomingMissileLauncher>(ChildActor);
		}

		if (Minigun != nullptr && HomingMissileLauncher != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("child actor was playerturret: %s"), *ChildActor->GetName());
			break; // Exit the loop since we found what we were looking for
		}
	}

	if (Minigun && Minigun->GetIsOverheated())
	{
		Overheating = true;
	}
	else if (Minigun->GetOverheatValue() < 0.2)
	{
		Overheating = false;
	}

	if (Overheating)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun name is not shooting : , %s"), *Minigun->GetName());
		Minigun->ReleaseTrigger();
		MinigunPulledTrigger = false;
	}
	else if (!MinigunPulledTrigger)
	{
		MinigunPulledTrigger = true;

		Minigun->PullTrigger();
		UE_LOG(LogTemp, Warning, TEXT("minigun name is shooting : , %s"), *Minigun->GetName());
	}

	AController* EnemyController = Cast<AController>(AIController);
	ensureMsgf(EnemyController != nullptr, TEXT("Enemy controller was null"));
	if (HomingMissileLauncher && !HomingMissileLauncher->GetIsOnCooldown() && HomingMissileLauncher->
		CheckTargetInRange(Enemy))
	{
		HomingMissileLauncher->PullTrigger();
		//MissilePulledTrigger = true;
	}
	else if (HomingMissileLauncher && HomingMissileLauncher->GetChargeAmount() == MissileChargeAmount)
	{
		HomingMissileLauncher->ReleaseTrigger();
	}
	else if (HomingMissileLauncher && (!HomingMissileLauncher->CheckTargetInRange(Enemy) || !HomingMissileLauncher->
		CheckTargetLineOfSight(EnemyController)))
	{
		HomingMissileLauncher->ReleaseTrigger();
	}
	if (Minigun == nullptr || Turret == nullptr || HomingMissileLauncher == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun or player turret or homing missile launcher was null"));
	}
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
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), InterpSpeed);
	Turret->SetActorRotation(NewRotation);
}

void AEnemyVehiclePawn::ManageSpeed()
{
	if (!MySpline) return;
	//UE_LOG(LogTemp, Warning, TEXT("maxspeed before calculation: %f "), MaxSpeed);
	//kolla rotationen till nästa spline point, adjust speed beroende på storleken av vinkel (större sväng ger större vinkel, Yaw specifikt) 
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
	//UE_LOG(LogTemp, Warning, TEXT("delta rotator %s"), *DeltaRotator.ToString());
	float DeltaYaw = FMath::Abs(DeltaRotator.Yaw);
	float ABSDeltaYaw = DeltaYaw;

	//5000 värdet kan lekas runt med, mindre värde ger mindre speed, högre värde ger mer speed. 
	DeltaYaw = 3000 / DeltaYaw;

	//slow down faster if max speed is bigger than delta yaw
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * MaxSpeed > DeltaYaw ? 20.0 : 0.1;

	MaxSpeed = FMath::Lerp(MaxSpeed, DeltaYaw, DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("maxspeed %f"), MaxSpeed);

	//clamp value betwwen low speed and maxspeed
	MaxSpeed = FMath::Clamp(MaxSpeed, 50, 800);
	//	UE_LOG(LogTemp, Warning, TEXT("maxspeed after clamp %f"), MaxSpeed);


	const float Speed = VehicleMovementComponent->GetForwardSpeed();
	UE_LOG(LogTemp, Warning, TEXT("forward speed %f"), VehicleMovementComponent->GetForwardSpeed());
	float TempBrakeInput = VehicleMovementComponent->GetBrakeInput();
	UE_LOG(LogTemp, Warning, TEXT("delta yaw value: %f"), ABSDeltaYaw);

	if (ABSDeltaYaw > 3 && VehicleMovementComponent->GetForwardSpeed() > 500)
	{
		VehicleMovementComponent->SetThrottleInput(0);
		UE_LOG(LogTemp, Warning, TEXT("in slowing down function: "));
		float MaxDeltaYaw = 30;
		float NormalizedDeltaYaw = FMath::Clamp(ABSDeltaYaw / MaxDeltaYaw, 0.0f, 1.0f);
		UE_LOG(LogTemp, Warning, TEXT("Normalized delta yaw: %f"), NormalizedDeltaYaw);
		float AIBrakeInput = NormalizedDeltaYaw;

		const float LerpValue = FMath::Lerp(TempBrakeInput, AIBrakeInput, GetWorld()->DeltaTimeSeconds * 80);

		VehicleMovementComponent->SetBrakeInput(LerpValue);
		UE_LOG(LogTemp, Warning, TEXT("lerp value brake input: %f"), LerpValue);
	}
	else if (Speed > MaxSpeed)
	{
		VehicleMovementComponent->SetBrakeInput(0);
		// be able to slow down very much faster in a curve
		VehicleMovementComponent->SetThrottleInput(VehicleMovementComponent->GetThrottleInput() - 0.2);
		//UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComponent->SetThrottleInput(0.6);
		VehicleMovementComponent->SetBrakeInput(0);
	}
}


void AEnemyVehiclePawn::AddNewTurretRotation()
{
	//add a new rotation to the target rotation
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f);
	RotationIncrement = FMath::RandBool() ? FRotator(0, 50, 0) : FRotator(0, -50, 0);
	StartingRotation = Turret->GetActorRotation();
	TargetRotation.Yaw = StartingRotation.Yaw + RotationIncrement.Yaw;
	TimerIsActive = false;
}


void AEnemyVehiclePawn::DriveAlongSpline()
{
	InitializeSpline();
	if (LeftSensor == nullptr || RightSensor == nullptr || VehicleMovementComponent == nullptr || MySpline == nullptr)
		return;
	//get a spline point along the spline
	//only gets the point if you are at the start point of the spline. Very few Work cases

	if (!HasNewSplineBeenSetup && MySpline)
	{
		UE_LOG(LogTemp, Warning, TEXT("setting spline direction"));
		FVector SplineStart = MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = MySpline->GetLocationAtSplinePoint(MySpline->GetNumberOfSplinePoints() - 1,
		                                                       ESplineCoordinateSpace::World);
		// Calculate distances from actor to start and end points
		float DistanceToStart = FVector::Dist(GetActorLocation(), SplineStart);
		float DistanceToEnd = FVector::Dist(GetActorLocation(), SplineEnd);
		if (DistanceToEnd > DistanceToStart)
		{
			GoToEndOfSpline = true;
		}
		else
		{
			GoToEndOfSpline = false;
		}
		HasNewSplineBeenSetup = true;

		float ClosestInputKey = MySpline->FindInputKeyClosestToWorldLocation(GetActorLocation());
		TargetSplineDistance = MySpline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);
		UE_LOG(LogTemp, Warning, TEXT("closest input key %f"), ClosestInputKey);
		UE_LOG(LogTemp, Warning, TEXT("target spline distance at input key %f"), ClosestInputKey);
	}


	//target spline distance set based on my actor location

	// Get the location of your actor

	//

	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);


	//check if car has reached point, then get a new point
	//implement so you can drive backwards along the spline
	float DistanceCheck = FVector::Dist(SplineLocationPoint, GetActorLocation());
	if (DistanceCheck < NextPointOnSplineThreshold)
	{
		GoToEndOfSpline ? TargetSplineDistance += 500 : TargetSplineDistance -= 500;
	}


	//steering
	FVector CurrentVelocity = GetVelocity();
	FVector PredictedLocation = GetActorLocation();
	//dela upp distansen till spline punkten i 5 bitar
	float DistBetweenPoint = FVector::Dist(PredictedLocation, SplineLocationPoint);
	float Time = DistBetweenPoint / CurrentVelocity.Length();
	float TimeStep = Time / 5;

	FVector TempVel = CurrentVelocity;
	TempVel.Normalize(0.0001);
	UE_LOG(LogTemp, Warning, TEXT("current vel:  %s"), *TempVel.ToString());
	FRotator VelRotator = UKismetMathLibrary::MakeRotFromX(TempVel);
	//få svängnings graden
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(VelRotator, GetActorRotation());
	float SteerYaw = DeltaRotator.Yaw;
	UE_LOG(LogTemp, Warning, TEXT("Steer yaw :  %f"), SteerYaw);
	for (int i = 0; i < 5; i++)
	{
		PredictedLocation = CurrentVelocity * TimeStep + PredictedLocation;
		CurrentVelocity.RotateAngleAxis(SteerYaw, FVector(0, 0, 1));
	}
	FRotator PredictedRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PredictedLocation);
	FRotator SplinePointRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	FRotator PredictedDeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(PredictedRotator, SplinePointRotator);
	float DeltaYaw = PredictedDeltaRotator.Yaw *= -0.1;
	float SteeringValue = FMath::Clamp(DeltaYaw, -1.0, 1.0);


	//check which sensor is closer to point
	SensorGapDifference = FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) - FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint);
	SensorGapDifference = FMath::Abs(SensorGapDifference);


	//steering
	//float TempSteeringInput = SteeringValue;

	/*if (SensorGapDifference < 10)
	{
		SteeringInput = 0;
	}*/
	/*else if (FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) > FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint))
	{
		SteeringInput = 0.6;
	}
	else
	{
		SteeringInput = -0.6;
	}*/
	//lerp the steering input for smoother turn
	// find out waht delta time * 40 does
	//const float LerpValue = FMath::Lerp(TempSteeringInput, SteeringInput, GetWorld()->DeltaTimeSeconds * 40);
	//lerp for smoother turning curve
	//TempSteeringInput = LerpValue;
	VehicleMovementComponent->SetSteeringInput(SteeringValue);
}

void AEnemyVehiclePawn::CheckIfAtEndOfSpline()
{
	if (!MySpline) return;
	
	if (GoToEndOfSpline
	? FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(MySpline->GetNumberOfSplinePoints() - 1,
													   ESplineCoordinateSpace::World)) < SplineEndPointDistanceThreshold
		    : FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World)) <
		    SplineEndPointDistanceThreshold)
	{
		VehicleMovementComponent->SetThrottleInput(0);
		VehicleMovementComponent->SetSteeringInput(0);
		BlackboardComp->SetValueAsBool("AtRoadEnd", true);
		HasNewSplineBeenSetup = false;
	}
}

bool AEnemyVehiclePawn::InitializeSensors()
{
	// kan säkert förenklas denna metod 
	const AActor* CarActor = Cast<AActor>(BlackboardComp->GetValueAsObject("ObjectCar"));

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

bool AEnemyVehiclePawn::InitializeSpline()
{
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("RoadSpline"));
	if (ActorRoadSpline == nullptr)
	{
		//	UE_LOG(LogTemp, Warning, TEXT("Spline Component not found on AIPawn., Actor roadspline was null"));
		return false;
	}

	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	if (MySpline == nullptr) return false;
	return true;
}

void AEnemyVehiclePawn::SetStartingRotation()
{
	StartingRotation = Turret->GetActorRotation();
}

void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

void AEnemyVehiclePawn::SetHasNewSplineBeenSetup(bool bValue)
{
	HasNewSplineBeenSetup = bValue;
}
