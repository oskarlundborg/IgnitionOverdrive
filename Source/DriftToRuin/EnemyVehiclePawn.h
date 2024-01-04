//Daniel Olsson - all kod för AI
//Mihajlo Radotic - Weapon setup in beginplay

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
	UFUNCTION(BlueprintCallable)
	void SetSwitchString(const FString& NewSwitchString);
	UFUNCTION(BlueprintCallable)
	void SetTickEnabledAI(bool bTickEnabled);
	UFUNCTION(BlueprintCallable)
	void ResetPulledTriggerValues(bool bPulledTrigger);
	
	void SetHasNewSplineBeenSetup(bool bValue);

	//get functions 
	UFUNCTION(BlueprintCallable)
	AAITurret* GetAITurret() const;
	FTimerHandle& GetMissileTimerHandle();

	//events för ljud
	UFUNCTION(BlueprintImplementableEvent)
	void PlayMinigunSound();
	UFUNCTION(BlueprintImplementableEvent)
	void StopMinigunSound();
	
	
private:
	//weapon components
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AAITurret> AITurretClass;
	UPROPERTY()
	AAITurret* Turret;

	//AI components
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	UBlackboardComponent* BlackboardComp;
	UPROPERTY()
	class USplineComponent* MySpline = nullptr;
	
	//Sensors
	UPROPERTY()
	float SensorGapDifference;
	UPROPERTY()
	USceneComponent* LeftSensor;
	UPROPERTY()
	USceneComponent* RightSensor;
	
	// car driving
	UPROPERTY()
	float ThrottleInput;
	UPROPERTY()
	float BrakeInput;
	UPROPERTY()
	float SteeringInput;

	//Speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float DynamicMaxSpeed = 6000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float ClampedMaxSped = 8000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float ClampedMinSpeed = 2000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float MinSpeedAtLargeCurve = 2000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float EstimatedAverageSpeed = 3000.0f;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	int TurnSlowdownCurveThreshold = 12;
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float MaxDeltaYaw = 30;
	
	//Behavior //göra om till enumerator, hann ej
	UPROPERTY()
	FString SwitchString = "Drive";

	//spline values
	UPROPERTY()
	FVector SplineLocationPoint;
	UPROPERTY()
	float TargetSplineDistance;
	
	//pathfinding
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float SplineEndPointDistanceThreshold = 300;
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float NextPointOnSplineThreshold = 1000;

	UPROPERTY()
	bool bGoToEndOfSpline;
	UPROPERTY()
	bool bHasNewSplineBeenSetup = false;
	
	//rotation for turret
	UPROPERTY()
	FRotator TurretRotation;
	UPROPERTY()
	FRotator TargetRotation;
	UPROPERTY()
	FRotator NewRotation;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	float RotationInterpSpeed = 15.0f;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMinRange = 1;
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMaxRange = 2;

	//shoot
	UPROPERTY()
	bool bOverheating = false;

	//missile
	UPROPERTY()
	bool bMinigunPulledTrigger = false;
	UPROPERTY()
	bool bMissileIsAvailable = false;
	UPROPERTY()
	int32 MissileCharge;
	UPROPERTY()
	ABaseVehiclePawn* AIEnemy;

	UPROPERTY()
	bool bHasKilled = false;

	//timers
	UPROPERTY()
	bool bTimerIsActive = false;
	UPROPERTY()
	bool bTimerFirstTime = true;
	UPROPERTY()
	bool bHominIsActive = false;
	
	UPROPERTY()
	int TimeElapsed;
	UPROPERTY()
	int TurretDelayTime = FMath::RandRange(1.0f, 3.0f);
	UPROPERTY()
	float TurretChargeTime = 2.5f;
	
	UPROPERTY()
	FTimerHandle MissileCooldownTimer;
	UPROPERTY()
	FTimerHandle ChargeAndFireTimer;
	UPROPERTY()
	FTimerHandle TimerHandle_SetStartingRotation;
	UPROPERTY()
	FTimerHandle TimerHandle_ResetRotationFlag;

	//driving behavior functions
	void DrivePath();
	void DriveAndShoot();
	void ReactToGetShot();
	

	//speed 
	void ManageSpeed();
	void AdjustSpeedBasedOnLargerTurnCurve(float ABSDeltaYaw, float Speed, float TempBrakeInput) const;

	//spline behavior
	void DriveAlongSpline();
	void SetUpNewSpline();
	void SetGoToEndOfSpline(float DifferenceYawStartPoint, float DifferenceYawEndPoint);
	void CheckIfAtEndOfSpline();

	//turret rotation
	void RandomlyRotateTurret();
	void SetStartingRotation();
	void AddNewTurretRotation();
	void RotateTowardsShootingEnemy();

	//shoot minigun
	void ShootMinigun();
	void FireMinigun();
	//missile
	void FireHomingMissile(float DistToTarget);
	void FireLoadedMissile();

	//helper function
	bool InitializeSensors();
	bool InitializeSpline();
};
