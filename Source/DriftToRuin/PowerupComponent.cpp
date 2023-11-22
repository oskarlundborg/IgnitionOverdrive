// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupComponent.h"
#include "BaseVehiclePawn.h"
#include "Minigun.h"


// Sets default values for this component's properties
UPowerupComponent::UPowerupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UPowerupComponent::HealthPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}
}

void UPowerupComponent::BoostPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}
}

void UPowerupComponent::OverheatPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}

	AMinigun* Minigun = Owner->GetMinigun();
	Minigun->PoweredUp = true;
	Minigun->PowerAmmo = 100;
	Owner->SetMinigunDamage(Owner->GetMinigunDefaultDamage() * 2);
	OverHeatPowerupActive = true;
	//Aktivera progressbar för powered ammo ovanpå overheat bar
}

void UPowerupComponent::ClearPowerup()
{
	AMinigun* Minigun = Owner->GetMinigun();

	if (Owner == nullptr)
	{
		return;
	}

	switch (Owner->HeldPowerup)
	{
	case 3:
		Minigun->PoweredUp = false;
		Owner->SetMinigunDamage(Owner->GetMinigunDefaultDamage());
		OverHeatPowerupActive = false;
		break;
	default:
		break;
	}

	Owner->SetHeldPowerup(0);
}

// Called when the game starts
void UPowerupComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPowerupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

