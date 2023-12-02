// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupComponent.h"
#include "BaseVehiclePawn.h"
#include "Minigun.h"
#include "HealthComponent.h"


// Sets default values for this component's properties
UPowerupComponent::UPowerupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// 
//
//	ändra kanske owner nullcheck till ensure / egen macro
//
//

void UPowerupComponent::HealthPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}

	HealthPowerupActive = true;
	Owner->GetHealthComponent()->IsPoweredUp = true;
	Owner->GetHealthComponent()->RegenerateHealth();
}

void UPowerupComponent::BoostPowerup()
{

	if (Owner == nullptr)
	{
		return;
	}

	BoostPowerupActive = true;
	Owner->SetBoostCost(0);
	Owner->SetBoostAmount(Owner->GetMaxBoostAmount());

	FTimerHandle tempHandle;
	GetWorld()->GetTimerManager().SetTimer(tempHandle, this, &UPowerupComponent::ClearPowerup, BoostPowerDuration, false);
}

void UPowerupComponent::ShieldPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}

	ShieldPowerupActive = true;

	Owner->GetShieldMeshComponent()->SetVisibility(true);
	Owner->GetShieldMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Owner->GetShieldMeshComponent()->SetGenerateOverlapEvents(true);

	FTimerHandle tempHandle;
	GetWorld()->GetTimerManager().SetTimer(tempHandle, this, &UPowerupComponent::ClearPowerup, BoostPowerDuration, false);


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

	/*
	
	
		Ändra från switch
	
	
	*/

	if (Owner == nullptr)
	{
		return;
	}

	AMinigun* Minigun = Owner->GetMinigun();

	switch (Owner->HeldPowerup)
	{

	case 1:
		Owner->GetHealthComponent()->IsPoweredUp = false;
		HealthPowerupActive = false;
		break;

	case 2:
		Owner->ResetBoostCost();
		BoostPowerupActive = false;
		break;
	case 3:
		Minigun->PoweredUp = false;
		Owner->SetMinigunDamage(Owner->GetMinigunDefaultDamage());
		OverHeatPowerupActive = false;
		break;

	case 4:

		ShieldPowerupActive = false;
		Owner->GetShieldMeshComponent()->SetVisibility(false);
		Owner->GetShieldMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Owner->GetShieldMeshComponent()->SetGenerateOverlapEvents(false);

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

