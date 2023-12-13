// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PowerupComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DRIFTTORUIN_API UPowerupComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPowerupComponent();

	UFUNCTION()
	void HealthPowerup();

	UFUNCTION()
	void BoostPowerup();

	UFUNCTION()
	void ShieldPowerup();

	UFUNCTION()
	void OverheatPowerup();

	UFUNCTION()
	void ClearPowerup(int PowerupId);

	UFUNCTION(BlueprintCallable)
	void ResetPowerups();

	UFUNCTION(BlueprintCallable)
	float GetPowerupPercentage(float PowerupFloat, float PowerupDuration);

	class ABaseVehiclePawn* Owner;

	UPROPERTY(BlueprintReadOnly)
	bool bOverHeatPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool bHealthPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool bBoostPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool bShieldPowerupActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HealthPowerDuration = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BoostPowerDuration = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ShieldPowerDuration = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float OverheatPowerDuration = 10;

	UPROPERTY(BlueprintReadOnly)
	float BoostPowerTime = 0;
	UPROPERTY(BlueprintReadOnly)
	float HealthPowerTime = 0;
	UPROPERTY(BlueprintReadOnly)
	float OverheatPowerTime = 0;
	UPROPERTY(BlueprintReadOnly)
	float ShieldPowerTime = 0;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
