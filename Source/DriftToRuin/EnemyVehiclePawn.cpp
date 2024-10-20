//Daniel Olsson 
//Mihaljo Radotic

#include "EnemyVehiclePawn.h"
#include "AITurret.h"
#include "AIController.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "TimerManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SplineComponent.h"
#include "Kismet/KismetMathLibrary.h"

AEnemyVehiclePawn::AEnemyVehiclePawn()
{
}

void AEnemyVehiclePawn::BeginPlay()
{
	Super::BeginPlay(); // Call the parent class's BeginPlay method
	TurretDelayTime = FMath::RandRange(1.0f, 3.0f); // Initialize turret delay time with a random value

	// Check if the turret, minigun, or homing launcher classes are not set
	if (AITurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;

	// Spawn and attach the turret to the vehicle
	Turret = GetWorld()->SpawnActor<AAITurret>(AITurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);

	// Spawn and attach the minigun to the turret
	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("MinigunRef"));
	Minigun->SetOwner(this);
	Minigun->InitializeOwnerVariables(); // Initialize minigun variables

	// Spawn and attach the homing missile launcher to the turret
	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MissileLauncerRef"));
	HomingLauncher->SetOwner(this);
	HomingLauncher->InitializeOwnerVariables(); // Initialize homing launcher variables

	// Set the starting rotation of the turret
	SetStartingRotation();

	// Get the AI controller and blackboard component
	AIController = Cast<AAIController>(GetController());
	if (AIController != nullptr)
	{
		BlackboardComp = AIController->GetBlackboardComponent();
		ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr")); // Ensure the blackboard component is not null
	}
	BlackboardComp->SetValueAsString("StringBehavior", "Drive"); // Set the initial behavior to "Drive"

	// Ensure the vehicle movement component is not null
	ensureMsgf(VehicleMovementComp != nullptr, TEXT("Vehicle movement comp was null"));
	if (VehicleMovementComp == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("movement component null")); // Log a warning if the movement component is null
	}

	// Set the maximum angular velocity for the vehicle's physics
	VehicleMovementComp->UpdatedPrimitive->SetPhysicsMaxAngularVelocityInDegrees(180);

	// Initialize sensors and spline
	InitializeSensors();
	InitializeSpline();
}

void AEnemyVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Switch behavior based on the value of SwitchString, change to enum for better performance
	if (SwitchString == "DriveAndShoot")
	{
		DriveAndShoot(); // Execute DriveAndShoot behavior
	}
	else if (SwitchString == "Drive")
	{
		DrivePath(); // Execute DrivePath behavior
	}
	else if (SwitchString == "ReactToGetShot")
	{
		ReactToGetShot(); // Execute ReactToGetShot behavior
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("incorrect switch string, no behavior being set")); // Log an error if SwitchString is invalid
	}
}

// Set the behavior switch string
void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

// Enables or disables AI ticking, Callable function in blueprint
void AEnemyVehiclePawn::SetTickEnabledAI(bool bTickEnabled)
{
	Minigun->ReleaseTrigger();
	Minigun->SetActorTickEnabled(bTickEnabled);
	HomingLauncher->SetActorTickEnabled(bTickEnabled);
	SetActorTickEnabled(bTickEnabled);
}

//Reset values for minigun, Callable function in blueprint
void AEnemyVehiclePawn::ResetValues(bool pulledTrigger)
{
	bMinigunPulledTrigger = pulledTrigger;
	Minigun->ReleaseTrigger();
	bHominIsActive = false;
}
//reset behavior tree and blackboard, callable function in blueprint
void AEnemyVehiclePawn::ResetBTTree()
{
	UBehaviorTreeComponent* BehaviorTreeComponent = Cast<UBehaviorTreeComponent>(AIController->GetBrainComponent());
	if(BehaviorTreeComponent)
	{
		BehaviorTreeComponent->RestartTree();
	}
	SwitchString = "Drive";
	BlackboardComp->ClearValue("Enemy");
	BlackboardComp->ClearValue("EnemyLocation");
	Turret->SetActorRelativeRotation(FRotator(GetActorRotation().Pitch, 0, GetActorRotation().Roll));
	TargetRotation = FRotator(0, 0, 0);
	BlackboardComp->ClearValue("TempRoadSpline");
	BlackboardComp->ClearValue("AIOwnedRoadSpline");
}
//set if new spline has been setup
void AEnemyVehiclePawn::SetHasNewSplineBeenSetup(bool bValue)
{
	bHasNewSplineBeenSetup = bValue;
}
//get turret
AAITurret* AEnemyVehiclePawn::GetAITurret() const
{
	return Turret;
}
//get missile timer handle
FTimerHandle& AEnemyVehiclePawn::GetMissileTimerHandle()
{
	return ChargeAndFireTimer;
}
//regular Drive behavior
void AEnemyVehiclePawn::DrivePath()
{
	
	StopMinigunSound();

	RandomlyRotateTurret();

	DriveAlongSpline();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}
//Drive and shoot behavior
void AEnemyVehiclePawn::DriveAndShoot()
{
	DriveAlongSpline();

	ShootMinigun();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}
//React to get shot behavior
void AEnemyVehiclePawn::ReactToGetShot()
{
	StopMinigunSound();

	RotateTowardsShootingEnemy();

	DriveAlongSpline();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}

//Manage speed during driving
void AEnemyVehiclePawn::ManageSpeed()
{
	// Check if the spline is null
	if (MySpline == nullptr)
	{
		return;
	}

	// Get the rotation of the turn curve
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
	float DeltaYaw = FMath::Abs(DeltaRotator.Yaw);
	float ABSDeltaYaw = DeltaYaw;

	// Calculate DeltaYaw based on the estimated average speed
	DeltaYaw = EstimatedAverageSpeed / DeltaYaw;

	// Slow down faster if max speed is bigger than delta yaw
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * DynamicMaxSpeed > DeltaYaw ? 30.0 : 0.1;

	// Dynamically change max speed based on the turn curve
	DynamicMaxSpeed = FMath::Lerp(DynamicMaxSpeed, DeltaYaw, DeltaTime);
	DynamicMaxSpeed = FMath::Clamp(DynamicMaxSpeed, ClampedMinSpeed, ClampedMaxSped);

	// Get the current speed and brake input
	const float Speed = VehicleMovementComp->GetForwardSpeed();
	float TempBrakeInput = VehicleMovementComp->GetBrakeInput();

	// Adjust speed based on larger turn curve
	AdjustSpeedBasedOnLargerTurnCurve(ABSDeltaYaw, Speed, TempBrakeInput);
}

// Adjust the speed based on the larger turn curve
void AEnemyVehiclePawn::AdjustSpeedBasedOnLargerTurnCurve(float ABSDeltaYaw, const float Speed,
														  float TempBrakeInput) const
{
	// Check if the absolute delta yaw is greater than the threshold and the vehicle's forward speed is above the minimum speed at large curves
	if (ABSDeltaYaw > TurnSlowdownCurveThreshold && VehicleMovementComp->GetForwardSpeed() > MinSpeedAtLargeCurve)
	{
		// Slow down drastically if the turn curve angle is high
		VehicleMovementComp->SetThrottleInput(0);
		float NormalizedDeltaYaw = FMath::Clamp(ABSDeltaYaw / MaxDeltaYaw, 0.0f, 1.0f);
		float AIBrakeInput = NormalizedDeltaYaw;
		const float LerpValue = FMath::Lerp(TempBrakeInput, AIBrakeInput, GetWorld()->DeltaTimeSeconds * 80);
		VehicleMovementComp->SetBrakeInput(LerpValue);
	}
	// Check if the current speed is greater than the dynamic maximum speed
	else if (Speed > DynamicMaxSpeed)
	{
		// Slow down if the speed is above the maximum speed
		VehicleMovementComp->SetBrakeInput(0);
		VehicleMovementComp->SetThrottleInput(VehicleMovementComp->GetThrottleInput() - 0.2);
	}
	else
	{
		// Maintain a moderate throttle input if none of the above conditions are met
		VehicleMovementComp->SetThrottleInput(0.6);
		VehicleMovementComp->SetBrakeInput(0);
	}
}

// Drive along the spline path 
void AEnemyVehiclePawn::DriveAlongSpline()
{
	// Check if the spline, sensors, or vehicle movement component are null
	if (!InitializeSpline() || LeftSensor == nullptr || RightSensor == nullptr || VehicleMovementComp == nullptr ||
		MySpline == nullptr)
	{
		return;
	}

	// Set up a new spline if needed
	SetUpNewSpline();

	// Get the location point on the spline at the target distance
	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	// Check if the car has reached the point, then get a new point along the spline
	float DistanceCheck = FVector::Dist(SplineLocationPoint, GetActorLocation());
	if (DistanceCheck < NextPointOnSplineThreshold)
	{
		bGoToEndOfSpline ? TargetSplineDistance += 500 : TargetSplineDistance -= 500;
	}

	// Steering logic
	FVector CurrentVelocity = GetVelocity();
	FVector PredictedLocation = GetActorLocation();
	// Divide the distance to the point into 5 parts for a better turn curve
	float DistBetweenPoint = FVector::Dist(PredictedLocation, SplineLocationPoint);
	float Time = DistBetweenPoint / CurrentVelocity.Length();
	float TimeStep = Time / 5;

	// Normalize the current velocity
	FVector TempVel = CurrentVelocity;
	TempVel.Normalize(0.0001);
	FRotator VelRotator = UKismetMathLibrary::MakeRotFromX(TempVel);

	// Get the turning degree
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(VelRotator, GetActorRotation());
	float SteerYaw = DeltaRotator.Yaw;
	for (int i = 0; i < 5; i++)
	{
		PredictedLocation = CurrentVelocity * TimeStep + PredictedLocation;
		CurrentVelocity.RotateAngleAxis(SteerYaw, FVector(0, 0, 1));
	}

	// Set the steering value based on yaw rotation to the next spline point
	FRotator PredictedRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PredictedLocation);
	FRotator SplinePointRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	FRotator PredictedDeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(PredictedRotator, SplinePointRotator);
	float DeltaYaw = PredictedDeltaRotator.Yaw *= -0.1;
	float SteeringValue = FMath::Clamp(DeltaYaw, -1.0, 1.0);

	// Set the steering input for the vehicle movement component
	VehicleMovementComp->SetSteeringInput(SteeringValue);
}

//sets up a new spline to drive along
void AEnemyVehiclePawn::SetUpNewSpline()
{
	// Check if a new spline has not been set up and MySpline is valid
	if (!bHasNewSplineBeenSetup && MySpline)
	{
		// Get the start and end locations of the spline
		FVector SplineStart = MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = MySpline->GetLocationAtSplinePoint(MySpline->GetNumberOfSplinePoints() - 1,
		                                                       ESplineCoordinateSpace::World);
		// Calculate the rotation to the start and end points of the spline
		FRotator RotationToStartPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineStart);
		FRotator RotationToEndPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineEnd);

		// Calculate the yaw differences between the actor's rotation and the rotations to the start and end points
		float DifferenceYawStartPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToStartPoint.Yaw));
		float DifferenceYawEndPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToEndPoint.Yaw));

		// Set the flag to go to the end of the spline based on the yaw differences
		SetGoToEndOfSpline(DifferenceYawStartPoint, DifferenceYawEndPoint);

		// Find the closest input key on the spline to the actor's location and set the target spline distance
		float ClosestInputKey = MySpline->FindInputKeyClosestToWorldLocation(GetActorLocation());
		TargetSplineDistance = MySpline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);
	}
}

//Decide whether to go to the start or end of the spline based on car entry point 
void AEnemyVehiclePawn::SetGoToEndOfSpline(float DifferenceYawStartPoint, float DifferenceYawEndPoint)
{
	// Check if the yaw difference to the start point is greater than or equal to the yaw difference to the end point
	if (DifferenceYawStartPoint >= DifferenceYawEndPoint)
	{
		// Set the flag to go to the end of the spline
		bGoToEndOfSpline = true;
	}
	else
	{
		// Set the flag to go to the start of the spline
		bGoToEndOfSpline = false;
	}
	// Mark that a new spline has been set up
	bHasNewSplineBeenSetup = true;
}

//check if the vehicle is at the end of the spline and reset values if true
void AEnemyVehiclePawn::CheckIfAtEndOfSpline()
{
	// Check if the spline is null
	if (MySpline == nullptr)
	{
		return;
	}

	// Check if the vehicle is at the end of the spline or at the start of the spline
	if (bGoToEndOfSpline
		    ? FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(
			                    MySpline->GetNumberOfSplinePoints() - 1,
			                    ESplineCoordinateSpace::World)) < SplineEndPointDistanceThreshold
		    : FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World)) <
		    SplineEndPointDistanceThreshold)
	{
		// Reset values and add LastRoadSpline as TempRoadSpline in Blackboard
		BlackboardComp->SetValueAsObject("TempRoadSpline", BlackboardComp->GetValueAsObject("RoadSpline"));
		BlackboardComp->ClearValue("RoadSpline");
		BlackboardComp->SetValueAsBool("AtRoadEnd", true);
		VehicleMovementComp->SetSteeringInput(0);
		MySpline = nullptr;
		bHasNewSplineBeenSetup = false;
		TargetSplineDistance = 0.0f;
	}
}
//rotate smoothly the turret randomly during driving (searching for enemy)
void AEnemyVehiclePawn::RandomlyRotateTurret()
{
	// Rotate after a certain delay
	TimeElapsed = GetWorld()->GetTimeSeconds();
	if (!bTimerIsActive)
	{
		bTimerIsActive = true;
		// Set a timer to add a new turret rotation
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetRotationFlag, this,
		                                       &AEnemyVehiclePawn::AddNewTurretRotation,
		                                       bTimerFirstTime ? 0.6f : TurretDelayTime, false);
		if (bTimerFirstTime) bTimerFirstTime = false;
	}
	// Smooth rotation interpolation
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	Turret->SetActorRotation(NewRotation);
}

//set up starting rotation for turret
void AEnemyVehiclePawn::SetStartingRotation()
{
	TurretRotation = Turret->GetActorRotation();
}


//adds a new random turret rotation upon timer finished that is created and interpolated smoothly in ( RandomlyRotateTurret())  
void AEnemyVehiclePawn::AddNewTurretRotation()
{
	// Add a new rotation to the target rotation
	TurretDelayTime = FMath::RandRange(TurretDelayTimeMinRange, TurretDelayTimeMaxRange);
	const FRotator CarRotation = GetActorRotation();
	TurretRotation = Turret->GetActorRotation();

	// Generate random values for rotation
	const bool random = FMath::RandBool();
	const float RandomFloat = FMath::RandRange(0.0f, 1.0f);
	const float RandomYawFloatCar = FMath::RandRange(30, 70);
	const float RandomYawFloatTurret = FMath::RandRange(60, 150);

	// Calculate rotation increment based on car rotation
	const float RotationIncrementCar = random
		                                   ? CarRotation.Yaw - RandomYawFloatCar
		                                   : CarRotation.Yaw + RandomYawFloatCar;
	// Calculate rotation increment based on turret rotation
	const float RotationIncrementTurret = random
		                                      ? TurretRotation.Yaw + RandomYawFloatTurret
		                                      : TurretRotation.Yaw - RandomYawFloatTurret;

	// Choose rotation type based on random float value
	TargetRotation.Yaw = RandomFloat > 0.3f ? RotationIncrementCar : RotationIncrementTurret;

	// Reset timer flag
	bTimerIsActive = false;
}

//rotate towards the focused enemy that shot the vehicle
void AEnemyVehiclePawn::RotateTowardsShootingEnemy()
{
	// Get the object representing the enemy that shot
	UObject* ShootingEnemy = BlackboardComp->GetValueAsObject("GotShotByEnemy");
	// Cast the object to an AActor
	AActor* ShootingEnemyActor = Cast<AActor>(ShootingEnemy);
	// Check if the cast was successful
	if (ShootingEnemyActor == nullptr)
	{
		// Log a warning if the cast failed
		UE_LOG(LogTemp, Warning, TEXT("shooting enemyactor nullptr"));
		return;
	}

	// Calculate the yaw rotation to look at the shooting enemy
	TargetRotation.Yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),
	                                                            ShootingEnemyActor->GetActorLocation()).Yaw;

	// Interpolate smoothly to the new rotation
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	// Set the turret's rotation if the turret is valid
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}
}

//start shooting minigun and homing actions
void AEnemyVehiclePawn::ShootMinigun()
{
	// Rotate towards enemy
	const FVector EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	TargetRotation.Yaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLocation).Yaw;
	if (Minigun == nullptr || HomingLauncher == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("null minigun or homin, returning"));
		return;
	}
	// Interpolate smoothly to the new rotation
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}

	// Get the enemy object from the blackboard
	UObject* EnemyObject = BlackboardComp->GetValueAsObject("Enemy");
	AIEnemy = Cast<ABaseVehiclePawn>(EnemyObject);

	// Fire the minigun
	FireMinigun();

	// Get the distance to the target
	float DistToTarget = GetDistanceTo(AIEnemy);

	// Fire the homing missile
	FireHomingMissile(DistToTarget);
}

//fire minigun 
void AEnemyVehiclePawn::FireMinigun()
{
	// Check if the AI enemy is null or dead and do actions accordingly
	if (AIEnemy == nullptr || (AIEnemy && AIEnemy->GetIsDead()/* && Minigun->GetIsFiring()*/))
	{
		UE_LOG(LogTemp, Warning, TEXT("enemy is dead or null"));
		SwitchString = "Drive";
		bMinigunPulledTrigger = false;
		Minigun->ReleaseTrigger();
		StopMinigunSound();
		return;
	}

	// Check if the minigun is overheating and the trigger is pulled and release the trigger
	if (Minigun->GetIsOverheated() && bMinigunPulledTrigger)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun overheating releasing trigger"));
		Minigun->ReleaseTrigger();
		bMinigunPulledTrigger = false;
		StopMinigunSound();
	}

	// Check if the minigun is not overheating and the trigger is not pulled and pull the trigger
	if (Minigun->GetOverheatValue() < 0.1 && !bMinigunPulledTrigger)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun not overheating, pulling trigger is shooting"));
		bMinigunPulledTrigger = true;
		Minigun->PullTrigger();
		PlayMinigunSound();
	}
}
//fire homing missile
void AEnemyVehiclePawn::FireHomingMissile(float DistToTarget)
{
	// Check if the homing launcher, AI enemy, and distance to target are valid
	if (HomingLauncher && AIEnemy && DistToTarget < HomingLauncher->GetTargetRange() 
		&& !HomingLauncher->GetIsOnCooldown() && HomingLauncher->GetChargeAmount() <= 0 && !bHominIsActive)
	{
		// Set the homing missile as active
		bHominIsActive = true;
		// Randomly set the missile charge amount
		MissileCharge = FMath::RandRange(1, 3);
		// Set a timer to fire the loaded missile after the turret charge time
		GetWorld()->GetTimerManager().SetTimer(
			ChargeAndFireTimer,
			this,
			&AEnemyVehiclePawn::FireLoadedMissile,
			TurretChargeTime, // Set this to the time you want for charging
			false);
	}
}
// Fire loaded missile after tinme has passed
void AEnemyVehiclePawn::FireLoadedMissile()
{
	// Set the homing missile as inactive
	bHominIsActive = false;

	// Get the distance to the target
	float DistToTarget = GetDistanceTo(AIEnemy);

	// Check if the distance to the target is within the homing launcher's range
	if (DistToTarget < HomingLauncher->GetTargetRange())
	{
		// Fire the homing missile at the AI enemy with the specified missile charge
		HomingLauncher->OnFireAI(AIEnemy, MissileCharge);
	}
}

// This method initializes the left and right sensors for the vehicle. These were not used in any later calculations, but perhaps in future development they could be used for obstacle avoidance

bool AEnemyVehiclePawn::InitializeSensors()
{
	// Get the car actor from the blackboard
	const AActor* CarActor = Cast<AActor>(BlackboardComp->GetValueAsObject("ObjectCar"));

	// Get the left and right sensors by tag
	TArray<UActorComponent*> LeftSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "LeftSensor");
	TArray<UActorComponent*> RightSensors = CarActor->GetComponentsByTag(USceneComponent::StaticClass(), "RightSensor");
	LeftSensor = LeftSensors.Num() > 0 ? Cast<USceneComponent>(LeftSensors[0]) : nullptr;
	RightSensor = RightSensors.Num() > 0 ? Cast<USceneComponent>(RightSensors[0]) : nullptr;

	// Check if the sensors are null and log a warning if they are
	if (LeftSensor == nullptr || RightSensor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Sensors were nullptr"));
		return false;
	}
	return true;
}

// Initializes the spline component for the vehicle
bool AEnemyVehiclePawn::InitializeSpline()
{
	// Get the actor representing the road spline from the blackboard
	const AActor* ActorRoadSpline = Cast<AActor>(BlackboardComp->GetValueAsObject("RoadSpline"));
	if (ActorRoadSpline == nullptr)
	{
		// Return false if the actor is null
		return false;
	}
	// Get the spline component from the actor
	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	if (MySpline == nullptr)
	{
		// Return false if the spline component is null
		return false;
	}
	// Return true if the spline component is successfully initialized
	return true;
}
