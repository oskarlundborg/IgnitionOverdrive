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

	//switch med enum för strings, hann ej

	//välj körnings beteende
	if (SwitchString == "DriveAndShoot")
	{
		DriveAndShoot();
	}
	else if (SwitchString == "Drive")
	{
		DrivePath();
	}
	else if (SwitchString == "ReactToGetShot")
	{
		ReactToGetShot();
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("incorrect switch string, no behavior being set"));
	}
}

void AEnemyVehiclePawn::SetSwitchString(const FString& NewSwitchString)
{
	SwitchString = NewSwitchString;
}

void AEnemyVehiclePawn::SetTickEnabledAI(bool bTickEnabled)
{
	Minigun->ReleaseTrigger();
	Minigun->SetActorTickEnabled(bTickEnabled);
	HomingLauncher->SetActorTickEnabled(bTickEnabled);
	SetActorTickEnabled(bTickEnabled);
}

void AEnemyVehiclePawn::ResetPulledTriggerValues(bool pulledTrigger)
{
	bMinigunPulledTrigger = pulledTrigger;
	Minigun->ReleaseTrigger();
	bHominIsActive = false;
}

void AEnemyVehiclePawn::SetHasNewSplineBeenSetup(bool bValue)
{
	bHasNewSplineBeenSetup = bValue;
}

AAITurret* AEnemyVehiclePawn::GetAITurret() const
{
	return Turret;
}

FTimerHandle& AEnemyVehiclePawn::GetMissileTimerHandle()
{
	return ChargeAndFireTimer;
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
	DriveAlongSpline();

	ShootMinigun();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::ReactToGetShot()
{
	RotateTowardsShootingEnemy();

	DriveAlongSpline();

	ManageSpeed();

	CheckIfAtEndOfSpline();
}

void AEnemyVehiclePawn::ManageSpeed()
{
	if (!MySpline) return;
	//get rotation of turn curve
	const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotation, GetActorRotation());
	float DeltaYaw = FMath::Abs(DeltaRotator.Yaw);
	float ABSDeltaYaw = DeltaYaw;

	//3000 värdet kan lekas runt med, mindre värde ger mindre speed, högre värde ger mer speed. 
	DeltaYaw = 3000 / DeltaYaw;

	//slow down faster if max speed is bigger than delta yaw
	const float DeltaTime = GetWorld()->GetDeltaSeconds() * DynamicMaxSpeed > DeltaYaw ? 20.0 : 0.1;

	//dynamically change maxspeed based on turn curve
	DynamicMaxSpeed = FMath::Lerp(DynamicMaxSpeed, DeltaYaw, DeltaTime);
	DynamicMaxSpeed = FMath::Clamp(DynamicMaxSpeed, ClampedMinSpeed, ClampedMaxSped);

	const float Speed = VehicleMovementComp->GetForwardSpeed();
	float TempBrakeInput = VehicleMovementComp->GetBrakeInput();

	AdjustSpeedBasedOnLargerTurnCurve(ABSDeltaYaw, Speed, TempBrakeInput);
}

void AEnemyVehiclePawn::AdjustSpeedBasedOnLargerTurnCurve(float ABSDeltaYaw, const float Speed, float TempBrakeInput) const
{
	if (ABSDeltaYaw > TurnSlowdownCurveThreshold && VehicleMovementComp->GetForwardSpeed() > MinSpeedAtLargeCurve)
	{
		//slow down drastically if turn curve angle is high 
		VehicleMovementComp->SetThrottleInput(0);
		float NormalizedDeltaYaw = FMath::Clamp(ABSDeltaYaw / MaxDeltaYaw, 0.0f, 1.0f);
		float AIBrakeInput = NormalizedDeltaYaw;
		const float LerpValue = FMath::Lerp(TempBrakeInput, AIBrakeInput, GetWorld()->DeltaTimeSeconds * 80);
		VehicleMovementComp->SetBrakeInput(LerpValue);
	}
	else if (Speed > DynamicMaxSpeed)
	{
		//slowdown if speed is above maxspeed
		VehicleMovementComp->SetBrakeInput(0);
		VehicleMovementComp->SetThrottleInput(VehicleMovementComp->GetThrottleInput() - 0.2);
	}
	else
	{
		VehicleMovementComp->SetThrottleInput(0.6);
		VehicleMovementComp->SetBrakeInput(0);
	}
}


void AEnemyVehiclePawn::DriveAlongSpline()
{
	if (!InitializeSpline() || LeftSensor == nullptr || RightSensor == nullptr || VehicleMovementComp == nullptr ||
		MySpline == nullptr)
	{
		return;
	}

	SetUpNewSpline();

	SplineLocationPoint = MySpline->GetLocationAtDistanceAlongSpline(
		TargetSplineDistance, ESplineCoordinateSpace::World);

	//check if car has reached point, then get a new point along spline
	float DistanceCheck = FVector::Dist(SplineLocationPoint, GetActorLocation());
	if (DistanceCheck < NextPointOnSplineThreshold)
	{
		bGoToEndOfSpline ? TargetSplineDistance += 500 : TargetSplineDistance -= 500;
	}

	//steering
	FVector CurrentVelocity = GetVelocity();
	FVector PredictedLocation = GetActorLocation();
	//divide the distance to point into 5 parts
	float DistBetweenPoint = FVector::Dist(PredictedLocation, SplineLocationPoint);
	float Time = DistBetweenPoint / CurrentVelocity.Length();
	float TimeStep = Time / 5;

	FVector TempVel = CurrentVelocity;
	TempVel.Normalize(0.0001);
	FRotator VelRotator = UKismetMathLibrary::MakeRotFromX(TempVel);

	//get turning degree
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(VelRotator, GetActorRotation());
	float SteerYaw = DeltaRotator.Yaw;
	for (int i = 0; i < 5; i++)
	{
		PredictedLocation = CurrentVelocity * TimeStep + PredictedLocation;
		CurrentVelocity.RotateAngleAxis(SteerYaw, FVector(0, 0, 1));
	}
	//set steering value based on yaw rotation to next spline point
	FRotator PredictedRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), PredictedLocation);
	FRotator SplinePointRotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineLocationPoint);
	FRotator PredictedDeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(PredictedRotator, SplinePointRotator);
	float DeltaYaw = PredictedDeltaRotator.Yaw *= -0.1;
	float SteeringValue = FMath::Clamp(DeltaYaw, -1.0, 1.0);

	VehicleMovementComp->SetSteeringInput(SteeringValue);
}


void AEnemyVehiclePawn::SetUpNewSpline()
{
	if (!bHasNewSplineBeenSetup && MySpline)
	{
		FVector SplineStart = MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		FVector SplineEnd = MySpline->GetLocationAtSplinePoint(MySpline->GetNumberOfSplinePoints() - 1,
		                                                       ESplineCoordinateSpace::World);
		FRotator RotationToStartPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineStart);
		FRotator RotationToEndPoint = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), SplineEnd);

		float DifferenceYawStartPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToStartPoint.Yaw));
		float DifferenceYawEndPoint = FMath::Abs(
			FMath::Abs(GetActorRotation().Yaw) - FMath::Abs(RotationToEndPoint.Yaw));

		SetGoToEndOfSpline(DifferenceYawStartPoint, DifferenceYawEndPoint);

		float ClosestInputKey = MySpline->FindInputKeyClosestToWorldLocation(GetActorLocation());
		TargetSplineDistance = MySpline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);
	}
}

void AEnemyVehiclePawn::SetGoToEndOfSpline(float DifferenceYawStartPoint, float DifferenceYawEndPoint)
{
	if (DifferenceYawStartPoint >= DifferenceYawEndPoint)
	{
		bGoToEndOfSpline = true;
	}
	else
	{
		bGoToEndOfSpline = false;
	}
	bHasNewSplineBeenSetup = true;
}

void AEnemyVehiclePawn::CheckIfAtEndOfSpline()
{
	if (!MySpline) return;

	if (bGoToEndOfSpline
		    ? FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(
			                    MySpline->GetNumberOfSplinePoints() - 1,
			                    ESplineCoordinateSpace::World)) < SplineEndPointDistanceThreshold
		    : FVector::Dist(GetActorLocation(), MySpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World)) <
		    SplineEndPointDistanceThreshold)
	{
		//reset values and add LastRoadSpline as tempRoadSpline in BB
		BlackboardComp->SetValueAsObject("TempRoadSpline", MySpline);
		BlackboardComp->ClearValue("RoadSpline");
		BlackboardComp->SetValueAsBool("AtRoadEnd", true);
		bHasNewSplineBeenSetup = false;
		TargetSplineDistance = 0.0f;
	}
}

void AEnemyVehiclePawn::RandomlyRotateTurret()
{
	//rotera efter en viss delay
	TimeElapsed = GetWorld()->GetTimeSeconds();
	if (!bTimerIsActive)
	{
		bTimerIsActive = true;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ResetRotationFlag, this,
		                                       &AEnemyVehiclePawn::AddNewTurretRotation,
		                                       bTimerFirstTime ? 0.6f : TurretDelayTime, false);
		if (bTimerFirstTime) bTimerFirstTime = false;
	}
	//smooth rotation
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds() * 20, RotationInterpSpeed);
	Turret->SetActorRotation(NewRotation);
}

void AEnemyVehiclePawn::SetStartingRotation()
{
	TurretRotation = Turret->GetActorRotation();
}

void AEnemyVehiclePawn::AddNewTurretRotation()
{
	//add a new rotation to the target rotation
	TurretDelayTime = FMath::RandRange(TurretDelayTimeMinRange, TurretDelayTimeMaxRange);
	const FRotator CarRotation = GetActorRotation();
	TurretRotation = Turret->GetActorRotation();
	
	const bool random = FMath::RandBool();
	const float RandomFloat = FMath::RandRange(0.0f, 1.0f);
	const float RandomYawFloatCar = FMath::RandRange(30, 70);
	const float RandomYawFloatTurret = FMath::RandRange(60, 150);

	//rotation along car rotation
	const float RotationIncrementCar = random
		                                   ? CarRotation.Yaw - RandomYawFloatCar
		                                   : CarRotation.Yaw + RandomYawFloatCar;
	//rotation along turret rotation
	const float RotationIncrementTurret = random
		                                      ? TurretRotation.Yaw + RandomYawFloatTurret
		                                      : TurretRotation.Yaw - RandomYawFloatTurret;

	//choose rotation type based of bool value
	TargetRotation.Yaw = RandomFloat > 0.3f ? RotationIncrementCar : RotationIncrementTurret;

	bTimerIsActive = false;
}

void AEnemyVehiclePawn::RotateTowardsShootingEnemy()
{
	UObject* ShootingEnemy = BlackboardComp->GetValueAsObject("GotShotByEnemy");
	AActor* ShootingEnemyActor = Cast<AActor>(ShootingEnemy);
	if (ShootingEnemyActor == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("shooting enemyactor nullptr"));
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




void AEnemyVehiclePawn::ShootMinigun()
{
	//rotate towards enemy
	const FVector EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), EnemyLocation);
	if (Minigun == nullptr || HomingLauncher == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("null minigun or homin, returning"));
		return;
	}
	NewRotation = FMath::RInterpTo(Turret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), RotationInterpSpeed);
	if (Turret != nullptr)
	{
		Turret->SetActorRotation(NewRotation);
	}
	
	UObject* EnemyObject = BlackboardComp->GetValueAsObject("Enemy");
	AIEnemy = Cast<ABaseVehiclePawn>(EnemyObject);
	
	//funkar detta när en person är död, fortsätter den skjuta
	FireMinigun();
	
	float DistToTarget = GetDistanceTo(AIEnemy);
	
	FireHomingMissile(DistToTarget);
}

void AEnemyVehiclePawn::FireMinigun()
{
	if (AIEnemy && AIEnemy->GetIsDead() && Minigun->GetIsFiring())
	{
		UE_LOG(LogTemp, Warning, TEXT("enemy is dead"));
		bMinigunPulledTrigger = false;
		Minigun->ReleaseTrigger();
		return;
	}
	
	if (Minigun->GetIsOverheated() && bMinigunPulledTrigger)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun overheating releasing trigger"));
		Minigun->ReleaseTrigger();
		bMinigunPulledTrigger = false;
	}
	if (Minigun->GetOverheatValue() < 0.1 && !bMinigunPulledTrigger)
	{
		bMinigunPulledTrigger = true;
		Minigun->PullTrigger();
		UE_LOG(LogTemp, Warning, TEXT("minigun not overheating, pulling trigger is shooting"));
	}
}

void AEnemyVehiclePawn::FireHomingMissile(float DistToTarget)
{
	if (HomingLauncher && AIEnemy && DistToTarget < HomingLauncher->
		GetTargetRange() && !HomingLauncher
		->GetIsOnCooldown() && HomingLauncher->GetChargeAmount() <= 0 && !bHominIsActive)
	{
		bHominIsActive = true;
		MissileCharge = FMath::RandRange(1, 3);
		GetWorld()->GetTimerManager().SetTimer(
			ChargeAndFireTimer,
			this,
			&AEnemyVehiclePawn::FireLoadedMissile,
			TurretChargeTime, // Set this to the time you want for charging
			false);
	}
}

void AEnemyVehiclePawn::FireLoadedMissile()
{
	bHominIsActive = false;
	float DistToTarget = GetDistanceTo(AIEnemy);
	if (DistToTarget < HomingLauncher->GetTargetRange())
	{
		HomingLauncher->OnFireAI(AIEnemy, MissileCharge);
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
		return false;
	}
	MySpline = ActorRoadSpline->GetComponentByClass<USplineComponent>();
	if (MySpline == nullptr)
	{
		return false;
	}
	return true;
}
