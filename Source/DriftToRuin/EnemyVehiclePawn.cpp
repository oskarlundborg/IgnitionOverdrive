// Fill out your copyright notice in the Description page of Project Settings.
//Daniel Olsson AI engineer, behaviors i en switch form. beingplay setup är samma för player, dvs  (mihaljos weapon kod)


#include "EnemyVehiclePawn.h"
#include "AITurret.h"
#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "TimerManager.h"
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

	if (AITurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;
	Turret = GetWorld()->SpawnActor<AAITurret>(AITurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);


	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::KeepRelativeTransform,
	                           TEXT("MinigunRef"));
	Minigun->SetOwner(this);
	Minigun->InitializeOwnerVariables();

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	                                  TEXT("MissileLauncerRef"));
	HomingLauncher->SetOwner(this);
	HomingLauncher->InitializeOwnerVariables();

	// set turret starting location check if this works or i need the timer 
	SetStartingRotation();

	//get common components
	AIController = Cast<AAIController>(GetController());
	if (AIController != nullptr)
	{
		BlackboardComp = AIController->GetBlackboardComponent();
		ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	}
	BlackboardComp->SetValueAsString("StringBehavior", "Drive");

	//VehicleMovementComp = Cast<UChaosVehicleMovementComponent>(GetMovementComponent());
	ensureMsgf(VehicleMovementComp != nullptr, TEXT("Vehicle movement comp was null"));
	if (VehicleMovementComp == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement cojmponent null"));
	}

	VehicleMovementComp->UpdatedPrimitive->SetPhysicsMaxAngularVelocityInDegrees(180);

	//DefaultFrontFriction=VehicleMovementComp->Wheels[0]->FrictionForceMultiplier;

	//DefaultRearFriction=VehicleMovementComp->Wheels[2]->FrictionForceMultiplier;

	InitializeSensors();
	InitializeSpline();
}

void AEnemyVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//switch med enum för strings

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
	else if (SwitchString == "ReactToGetShot")
	{
		ReactToGetShot();
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

void AEnemyVehiclePawn::ReactToGetShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Reacting to get shot"));

	RotateTowardsShootingEnemy();
	DriveAlongSpline();
	ManageSpeed();
	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::Shoot()
{
	//UE_LOG(LogTemp, Warning, TEXT("AI player shooting and rotating to turret, bullet now."));
	const FVector EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLocation);

	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}
	ABaseVehiclePawn* Enemy = Cast<ABaseVehiclePawn>(BlackboardComp->GetValueAsObject("Enemy"));

	/*if (Enemy && Enemy->GetIsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("enemy is dead"));
		HasKilled = true;
		return;
	}*/

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
		if (HomingLauncher == nullptr)
		{
			HomingLauncher = Cast<AHomingMissileLauncher>(ChildActor);
		}

		if (Minigun != nullptr && HomingLauncher != nullptr)
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

	//homin missiles

	/*
	AController* EnemyController = Cast<AController>(AIController);
	ensureMsgf(EnemyController != nullptr, TEXT("Enemy controller was null"));*/
	UObject* EnemyObject = BlackboardComp->GetValueAsObject("Enemy");
	AIEnemy = Cast<AActor>(EnemyObject);
	float DistToTarget = GetDistanceTo(AIEnemy);
	//charge time needs to be done.

	if (HomingLauncher && AIEnemy && DistToTarget < HomingLauncher->
		GetTargetRange() && !HomingLauncher
		->GetIsOnCooldown() && HomingLauncher->GetChargeAmount() <= 0 && !HominIsActive)
	{
		HominIsActive = true;
		MissileCharge = FMath::RandRange(1, 3);

		//FTimerHandle ChargeAndFireTimer;
		GetWorld()->GetTimerManager().SetTimer(
			ChargeAndFireTimer,
			this,
			&AEnemyVehiclePawn::FireLoadedMissile,
			TurretChargeTime, // Set this to the time you want for charging
			false);

		//HomingLauncher->OnFireAI(AIEnemy, MissileCharge);
	}

	if (Minigun == nullptr || Turret == nullptr || HomingLauncher == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun or player turret or homing missile launcher was null"));
	}
}

void AEnemyVehiclePawn::FireLoadedMissile()
{
	HominIsActive = false;
	UE_LOG(LogTemp, Error, TEXT("about to shoot homin"));
	float DistToTarget = GetDistanceTo(AIEnemy);
	if (DistToTarget < HomingLauncher->GetTargetRange())
	{
		HomingLauncher->OnFireAI(AIEnemy, MissileCharge);
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
	                               GetWorld()->GetDeltaSeconds() * 20, RotationInterpSpeed);
	Turret->SetActorRotation(NewRotation);
}

void AEnemyVehiclePawn::AddNewTurretRotation()
{
	//add a new rotation to the target rotation
	TurretDelayTime = FMath::RandRange(TurretDelayTimeMinRange, TurretDelayTimeMaxRange);
	const FRotator CarRotation = GetActorRotation();
	TurretRotation = Turret->GetActorRotation();
	const float RandomValue = FMath::FRand(); // Generates a random float between 0 and 1

	// 70% chance to rotate towards car's rotation, 30% chance to rotate the other way
	//this rotation does not go thorugh - and 0 + values. it cant rotate around the 0 point.
	const bool random = FMath::RandBool();
	const float RandomFloat = FMath::RandRange(0.0f,1.0f);
	const float RandomYawFloatCar= FMath::RandRange(60, 70);
	const float RandomYawFloatTurret= FMath::RandRange(60, 150);
	
	const float RotationIncrementCar = random ? CarRotation.Yaw - RandomYawFloatCar : CarRotation.Yaw + RandomYawFloatCar;
	const float RotationIncrementTurret = random ? TurretRotation.Yaw + RandomYawFloatTurret : TurretRotation.Yaw - RandomYawFloatTurret;
	
	TargetRotation.Yaw = RandomFloat > 0.3f ? RotationIncrementCar : RotationIncrementTurret;

	TimerIsActive = false;
}

void AEnemyVehiclePawn::RotateTowardsShootingEnemy()
{
	UObject* ShootingEnemy = BlackboardComp->GetValueAsObject("GotShotByEnemy");
	AActor* ShootingEnemyActor = Cast<AActor>(ShootingEnemy);
	if (ShootingEnemyActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("shootingenemyactor nullptr"));
		return;
	}

	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), ShootingEnemyActor->GetActorLocation());

	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds() * 20, RotationInterpSpeed);
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}
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
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * DynamicMaxSpeed > DeltaYaw ? 20.0 : 0.1;

	DynamicMaxSpeed = FMath::Lerp(DynamicMaxSpeed, DeltaYaw, DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("maxspeed %f"), MaxSpeed);

	//clamp value betwwen low speed and maxspeed
	DynamicMaxSpeed = FMath::Clamp(DynamicMaxSpeed, ClampedMinSpeed, ClampedMaxSped);
	//	UE_LOG(LogTemp, Warning, TEXT("maxspeed after clamp %f"), MaxSpeed);

	const float Speed = VehicleMovementComp->GetForwardSpeed();
	//UE_LOG(LogTemp, Warning, TEXT("forward speed %f"), VehicleMovementComponent->GetForwardSpeed());
	float TempBrakeInput = VehicleMovementComp->GetBrakeInput();
	//	UE_LOG(LogTemp, Warning, TEXT("delta yaw value: %f"), ABSDeltaYaw);

	if (ABSDeltaYaw > 7 && VehicleMovementComp->GetForwardSpeed() > SpeedValueToDrasticallySlowDownInACurve)
	{
		VehicleMovementComp->SetThrottleInput(0);
		//	UE_LOG(LogTemp, Warning, TEXT("in slowing down function: "));
		float MaxDeltaYaw = 30;
		float NormalizedDeltaYaw = FMath::Clamp(ABSDeltaYaw / MaxDeltaYaw, 0.0f, 1.0f);
		//	UE_LOG(LogTemp, Warning, TEXT("Normalized delta yaw: %f"), NormalizedDeltaYaw);
		float AIBrakeInput = NormalizedDeltaYaw;

		const float LerpValue = FMath::Lerp(TempBrakeInput, AIBrakeInput, GetWorld()->DeltaTimeSeconds * 80);

		VehicleMovementComp->SetBrakeInput(LerpValue);
		//	UE_LOG(LogTemp, Warning, TEXT("lerp value brake input: %f"), LerpValue);
	}
	else if (Speed > DynamicMaxSpeed)
	{
		VehicleMovementComp->SetBrakeInput(0);
		// be able to slow down very much faster in a curve
		VehicleMovementComp->SetThrottleInput(VehicleMovementComp->GetThrottleInput() - 0.2);
		//UE_LOG(LogTemp, Warning, TEXT("throttle input changing to: %f"), VehicleMovementComponent->GetThrottleInput());
	}
	else
	{
		VehicleMovementComp->SetThrottleInput(0.6);
		VehicleMovementComp->SetBrakeInput(0);
	}
}


void AEnemyVehiclePawn::DriveAlongSpline()
{
	InitializeSpline();
	if (LeftSensor == nullptr || RightSensor == nullptr || VehicleMovementComp == nullptr || MySpline == nullptr)
		return;
	//get a spline point along the spline
	//only gets the point if you are at the start point of the spline. Very few Work cases

	if (!HasNewSplineBeenSetup && MySpline)
	{
		//UE_LOG(LogTemp, Warning, TEXT("setting spline direction"));
		FVector SplineStart = MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = MySpline->GetLocationAtSplinePoint(MySpline->GetNumberOfSplinePoints() - 1,
		                                                       ESplineCoordinateSpace::World);
		// Calculate rotation to start and end pouint, choose the point that has less rotation, so that car smoothly transitions into new spline road.

		// dubbel kolla dessa 
		FRotator RotationToStartPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineStart);
		FRotator RotationToEndPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineEnd);
		/*float DistanceToStart = FVector::Dist(GetActorLocation(), SplineStart);
		float DistanceToEnd = FVector::Dist(GetActorLocation(), SplineEnd);*/
		//FMath::Abs(RotationToEndPoint);
		//FMath::Abs(RotationToStartPoint);
		/*UE_LOG(LogTemp, Error, TEXT("RotationToEndPoint:  %s"), *RotationToEndPoint.ToString());
		UE_LOG(LogTemp, Error, TEXT("RotationToStartPoint: %s"), *RotationToStartPoint.ToString());
		UE_LOG(LogTemp, Error, TEXT("actor rotatation: %s"), *GetActorRotation().ToString());*/

		//float DifferenceYawStartPoint = FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToStartPoint.Yaw);
		//float DifferenceYawEndPoint = FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToEndPoint.Yaw);
		float DifferenceYawStartPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToStartPoint.Yaw));
		float DifferenceYawEndPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToEndPoint.Yaw));

		if (DifferenceYawStartPoint >= DifferenceYawEndPoint)
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
	//UE_LOG(LogTemp, Warning, TEXT("current vel:  %s"), *TempVel.ToString());
	FRotator VelRotator = UKismetMathLibrary::MakeRotFromX(TempVel);
	//få svängnings graden
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(VelRotator, GetActorRotation());
	float SteerYaw = DeltaRotator.Yaw;
	//UE_LOG(LogTemp, Warning, TEXT("Steer yaw :  %f"), SteerYaw);
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


	/*//check which sensor is closer to point
	SensorGapDifference = FVector::Dist(LeftSensor->GetComponentLocation(), SplineLocationPoint) - FVector::Dist(
		RightSensor->GetComponentLocation(), SplineLocationPoint);
	SensorGapDifference = FMath::Abs(SensorGapDifference);*/

	VehicleMovementComp->SetSteeringInput(SteeringValue);
}

void AEnemyVehiclePawn::CheckIfAtEndOfSpline()
{
	if (!MySpline) return;

	if (GoToEndOfSpline
		    ? FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(
			                    MySpline->GetNumberOfSplinePoints() - 1,
			                    ESplineCoordinateSpace::World)) < SplineEndPointDistanceThreshold
		    : FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World)) <
		    SplineEndPointDistanceThreshold)
	{
		//clear blackboard value
		//maybe set temp value
		BlackboardComp->SetValueAsObject("TempRoadSpline", MySpline);
		BlackboardComp->ClearValue("RoadSpline");

		VehicleMovementComp->SetThrottleInput(0);
		VehicleMovementComp->SetSteeringInput(0);
		BlackboardComp->SetValueAsBool("AtRoadEnd", true);
		HasNewSplineBeenSetup = false;
		TargetSplineDistance = 0.0f;
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
	TurretRotation = Turret->GetActorRotation();
}

void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

void AEnemyVehiclePawn::SetHasNewSplineBeenSetup(bool bValue)
{
	HasNewSplineBeenSetup = bValue;
}

void AEnemyVehiclePawn::SetTickEnabledAI(bool bTickEnabled)
{
	// kanske inte behövs 
	Minigun->ReleaseTrigger();
	Minigun->SetActorTickEnabled(bTickEnabled);
	HomingLauncher->SetActorTickEnabled(bTickEnabled);
	SetActorTickEnabled(bTickEnabled);
}

FTimerHandle& AEnemyVehiclePawn::GetMissileTimerHandle()
{
	return ChargeAndFireTimer;
}
