// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseVehiclePawn.h"
#include "PlayerVehiclePawn.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API APlayerVehiclePawn : public ABaseVehiclePawn
{
	GENERATED_BODY()

public:
	
	APlayerVehiclePawn();
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	float GetMinigunOverheatPercent() const;

	UFUNCTION(BlueprintCallable)
	bool GetMinigunIsOverheated() const;

	UFUNCTION(BlueprintCallable)
	float GetMissileChargePercent() const;
	
	UFUNCTION(BlueprintCallable)
	bool GetHomingIsCharging() const;

	UFUNCTION(BlueprintCallable)
	int32 GetHomingChargeAmount() const;

	UFUNCTION(BlueprintCallable)
	APlayerTurret* GetTurret() const;

	UPROPERTY(BlueprintReadWrite)
	float Sensitivity;

protected:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Input)
	class UInputMappingContext* VehicleMappingContext;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Turret")
	TSubclassOf<APlayerTurret> PlayerTurretClass;
	UPROPERTY()
	APlayerTurret* Turret;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* BrakingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* SteeringAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* LookUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* LookAroundAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* HandbrakeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* AirRollYawAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* AirRollRollAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* AirRollPitchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* FireMinigunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* FireHomingMissilesAction;

	//How fast air rolls can be done
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=AirRoll, meta=(AllowPrivateAccess = "true"))
	float AirRollSensitivity = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Drift, meta=(AllowPrivateAccess = "true"))
	float DriftRearFrictionForceMultiplier = 3.0f;

	float DefaultRearFrictionForceMultiplier;
	
	bool bCanAirRoll = false;

	void SetCanAirRollTrue()
	{
		bCanAirRoll = true;
	}

	FTimerHandle AirRollTimer;
	
	void ApplyThrottle(const struct FInputActionValue& Value);
	void ApplyBraking(const  FInputActionValue& Value);
	void ApplySteering(const FInputActionValue& Value);

	void LookUp(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);

	void OnHandbrakePressed();
	void OnHandbrakeReleased();

	void ApplyAirRollYaw(const FInputActionValue& Value);
	void ApplyAirRollRoll(const FInputActionValue& Value);
	void ApplyAirRollPitch(const FInputActionValue& Value);

	void FireMinigun();
	void FireMinigunCompleted();

	void FireHomingMissiles();
	void FireHomingMissilesCompleted();
};
