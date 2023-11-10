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

protected:
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Input)
	class UInputMappingContext* VehicleMappingContext;

private:
	
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

	void ApplyThrottle(const struct FInputActionValue& Value);
	void ApplyBraking(const  FInputActionValue& Value);
	void ApplySteering(const FInputActionValue& Value);

	void LookUp(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	
};