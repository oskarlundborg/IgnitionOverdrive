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
	void ClearPowerup();

	class ABaseVehiclePawn* Owner;

	UPROPERTY(BlueprintReadOnly)
	bool OverHeatPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool HealthPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool BoostPowerupActive = false;
	UPROPERTY(BlueprintReadOnly)
	bool ShieldPowerupActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HealthPowerDuration = 10;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BoostPowerDuration = 10;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
