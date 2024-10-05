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

    // Set functions
    UFUNCTION(BlueprintCallable)
    void SetSwitchString(const FString& NewSwitchString);  // Sets the behavior switch string
    UFUNCTION(BlueprintCallable)
    void SetTickEnabledAI(bool bTickEnabled); // Enables or disables AI ticking
	void SetHasNewSplineBeenSetup(bool bValue); // Sets whether a new spline has been set up

    // Resets certain Minigun values
    UFUNCTION(BlueprintCallable)
    void ResetValues(bool bPulledTrigger);
    // Resets the behavior tree
    UFUNCTION(BlueprintCallable)
    void ResetBTTree();


    // Get functions
    UFUNCTION(BlueprintCallable)
    AAITurret* GetAITurret() const; // Returns the AI turret
	FTimerHandle& GetMissileTimerHandle(); // Returns the missile timer handle

    // Events for sound
    UFUNCTION(BlueprintImplementableEvent) // Plays the minigun sound
    void PlayMinigunSound();
    UFUNCTION(BlueprintImplementableEvent)    // Stops the minigun sound
    void StopMinigunSound();
	
private:

	// Weapon components
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<AAITurret> AITurretClass; // Class of the AI turret
	UPROPERTY()
	AAITurret* Turret; // Instance of the AI turret

	// AI components
	UPROPERTY()
	AAIController* AIController; // AI controller for the vehicle
	UPROPERTY()
	UBlackboardComponent* BlackboardComp; // Blackboard component for AI behavior
	UPROPERTY()
	class USplineComponent* MySpline = nullptr; // Spline component for path following

	// Sensors
	UPROPERTY()
	float SensorGapDifference; // Difference in gap between sensors
	UPROPERTY()
	USceneComponent* LeftSensor; // Left sensor component
	UPROPERTY()
	USceneComponent* RightSensor; // Right sensor component

	// Car driving
	UPROPERTY()
	float ThrottleInput; // Throttle input value
	UPROPERTY()
	float BrakeInput; // Brake input value
	UPROPERTY()
	float SteeringInput; // Steering input value

		// Speed properties
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float DynamicMaxSpeed = 4000.0f; // Maximum dynamic speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float ClampedMaxSped = 6000.0f; // Maximum clamped speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float ClampedMinSpeed = 1500.0f; // Minimum clamped speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float MinSpeedAtLargeCurve = 1500.0f; // Minimum speed at large curves
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float EstimatedAverageSpeed = 3000.0f; // Estimated average speed
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	int TurnSlowdownCurveThreshold = 10; // Threshold for slowing down at turns
	UPROPERTY(EditDefaultsOnly, Category="Speed", meta=(AllowPrivateAccess=true))
	float MaxDeltaYaw = 30; // Maximum delta yaw for turns

	// Behavior properties
	UPROPERTY()
	FString SwitchString = "Drive"; // Current behavior switch string

	// Spline values
	UPROPERTY()
	FVector SplineLocationPoint; // Location point on the spline
	UPROPERTY()
	float TargetSplineDistance; // Target distance on the spline

	// Pathfinding properties
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float SplineEndPointDistanceThreshold = 800; // Distance threshold for the end of the spline
	UPROPERTY(EditDefaultsOnly, Category="Pathfinding|Spline", meta=(AllowPrivateAccess=true))
	float NextPointOnSplineThreshold = 1000; // Threshold for the next point on the spline

	UPROPERTY()
	bool bGoToEndOfSpline; // Flag to indicate if the vehicle should go to the end of the spline
	UPROPERTY()
	bool bHasNewSplineBeenSetup = false; // Flag to indicate if a new spline has been set up

	// Turret rotation properties
	UPROPERTY()
	FRotator TurretRotation; // Current rotation of the turret
	UPROPERTY()
	FRotator TargetRotation; // Target rotation for the turret
	UPROPERTY()
	FRotator NewRotation; // New rotation for the turret
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	float RotationInterpSpeed = 15.0f; // Interpolation speed for turret rotation
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMinRange = 1; // Minimum delay time for turret rotation
	UPROPERTY(EditDefaultsOnly, Category="Rotation", meta=(AllowPrivateAccess=true))
	int32 TurretDelayTimeMaxRange = 2; // Maximum delay time for turret rotation

	// Indicates if the minigun is overheating
	UPROPERTY()
	bool bOverheating = false;

	// Indicates if the minigun trigger is pulled
	UPROPERTY()
	bool bMinigunPulledTrigger = false;
	// Indicates if the missile is available
	UPROPERTY()
	bool bMissileIsAvailable = false;
	// Stores the missile charge amount
	UPROPERTY()
	int32 MissileCharge;
	// Pointer to the enemy vehicle pawn
	UPROPERTY()
	ABaseVehiclePawn* AIEnemy;

	// Indicates if the enemy has been killed
	UPROPERTY()
	bool bHasKilled = false;

	// Timer properties
	UPROPERTY()
	bool bTimerIsActive = false;
	UPROPERTY()
	bool bTimerFirstTime = true;
	// Indicates if the homing missile is active
	UPROPERTY()
	bool bHominIsActive = false;

	// Stores the elapsed time
	UPROPERTY()
	int TimeElapsed;
	// Delay time for turret rotation
	UPROPERTY()
	int TurretDelayTime = FMath::RandRange(1.0f, 3.0f);
	// Charge time for the turret
	UPROPERTY()
	float TurretChargeTime = 2.5f;

	// Timer handles for various actions
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
	

	// Speed functions to manage and adjust the vehicle's speed
	void ManageSpeed();
	void AdjustSpeedBasedOnLargerTurnCurve(float ABSDeltaYaw, float Speed, float TempBrakeInput) const;

	// Spline behavior functions to control the vehicle's movement along a spline
	void DriveAlongSpline();
	void SetUpNewSpline();
	void SetGoToEndOfSpline(float DifferenceYawStartPoint, float DifferenceYawEndPoint);
	void CheckIfAtEndOfSpline();

	// Turret rotation functions to control the turret's rotation
	void RandomlyRotateTurret();
	void SetStartingRotation();
	void AddNewTurretRotation();
	void RotateTowardsShootingEnemy();

	// Functions to control the minigun
	void ShootMinigun();
	void FireMinigun();

	// Functions to control the homing missile
	void FireHomingMissile(float DistToTarget);
	void FireLoadedMissile();

	// Helper functions to initialize sensors and spline
	bool InitializeSensors();
	bool InitializeSpline();
};
