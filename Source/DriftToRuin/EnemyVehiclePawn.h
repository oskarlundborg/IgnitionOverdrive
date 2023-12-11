// Fill out your copyright notice in the Description page of Project Settings.
//Daniel Olsson AI engineer, behaviors i en switch form. beingplay setup är samma för player, dvs  (mihaljos weapon kod)

#pragma once
#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.generated.h"

class AAITurret;
class UBehaviorTreeComponent;
class AAIController;
class UBlackboardComponent;
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AEnemyVehiclePawn : public ABaseVehiclePawn
{
	GENERATED_BODY()

public:
	AEnemyVehiclePawn();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	//set functions
	void SetSwitchString(const FString& NewSwitchString);
	void SetHasNewSplineBeenSetup(bool bValue);

	/*void RandomlyRotateTurret();
	void ManageSpeed();
	void DriveAlongSpline();
	void CheckIfAtEndOfSpline();


	//pathfinding
	UPROPERTY(EditAnywhere)
	float SplineEndPointDistanceThreshold = 800;
	UPROPERTY(EditAnywhere)
	float NextPointOnSplineThreshold = 1000;
	*/

	/*// car driving
	UPROPERTY(EditAnywhere)
	float ThrottleInput;
	UPROPERTY(EditAnywhere)
	float BrakeInput;
	UPROPERTY(EditAnywhere)
	float SteeringInput;
	UPROPERTY(EditAnywhere)
	float MaxSpeed = 1500.0f;*/

private:
	//weapon components
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AAITurret> AITurretClass;
	UPROPERTY()
	AAITurret* Turret;
	
	/*AMinigun* Minigun;
	UPROPERTY()
	AHomingMissileLauncher* HomingMissileLauncher;*/
	
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	UBlackboardComponent* BlackboardComp;
	UPROPERTY()
	class USplineComponent* MySpline = nullptr;
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	UChaosVehicleMovementComponent* AIVehicleMovementComp = nullptr;

	//pathfinding
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float SplineEndPointDistanceThreshold = 800;
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float NextPointOnSplineThreshold = 1000;

	// car driving
	UPROPERTY()
	float ThrottleInput;
	UPROPERTY()
	float BrakeInput;
	UPROPERTY()
	float SteeringInput;

	//Speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float MaxSpeed = 1500.0f;

	//Behavior //göra om till enumerator
	FString SwitchString = "Drive";

	//spline values
	UPROPERTY()
	FVector SplineLocationPoint;

	//FVector SplineTangent;

	UPROPERTY()
	bool GoToEndOfSpline;
	UPROPERTY()
	bool HasNewSplineBeenSetup = false;


	//rotation for turret
	UPROPERTY()
	FRotator TurretRotation;
	UPROPERTY()
	FRotator TargetRotation;
	UPROPERTY()
	FRotator NewRotation;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	float RotationInterpSpeed = 1;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMinRange = 1;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMaxRange = 3;

	//shoot

	UPROPERTY()
	bool Overheating = false;

	//missile
	UPROPERTY()
	bool MinigunPulledTrigger = false;
	UPROPERTY()
	bool MissileIsAvailable = false;
	UPROPERTY()
	int32 MissileCharge;
	UPROPERTY()
	AActor* AIEnemy;

	UPROPERTY()
	bool HasKilled = false;

	
	//timer // kan de göras om snyggare 
	UPROPERTY()
	bool TimerIsActive = false;
	UPROPERTY()
	bool TimerFirstTime = true;

	
	FTimerHandle MissileCooldownTimer;

	//Sensors
	UPROPERTY()
	float SensorGapDifference;
	UPROPERTY()
	USceneComponent* LeftSensor;
	UPROPERTY()
	USceneComponent* RightSensor;

	//timers - borde kollas igenom om dessa ska användas
	UPROPERTY()
	int TimeElapsed;
	UPROPERTY()
	int TurretDelayTime = FMath::RandRange(1.0f, 3.0f);

	UPROPERTY()
	FTimerHandle TimerHandle_SetStartingRotation;
	UPROPERTY()
	FTimerHandle TimerHandle_ResetRotationFlag;

	UPROPERTY()
	bool HominIsActive = false;
	//Functions

	FTimerHandle test;

	//driving behavior functions
	void DrivePath();
	void DriveAndShoot();

	//speed 
	void ManageSpeed();

	//spline behavior
	void DriveAlongSpline();
	void CheckIfAtEndOfSpline();

	//turret rotation
	void RandomlyRotateTurret();
	void SetStartingRotation();
	void AddNewTurretRotation();

	//shoot
	void Shoot();

	//missile
	void FireLoadedMissile();
	
	//helper function
	bool InitializeSensors();
	bool InitializeSpline();
};
