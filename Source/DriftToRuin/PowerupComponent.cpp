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

	bHealthPowerupActive = true;
	HealthPowerTime = HealthPowerDuration;
	Owner->GetHealthComponent()->IsPoweredUp = true;
	Owner->GetHealthComponent()->RegenerateHealth();
}

void UPowerupComponent::BoostPowerup()
{

	if (Owner == nullptr)
	{
		return;
	}

	bBoostPowerupActive = true;
	BoostPowerTime = BoostPowerDuration;
	Owner->SetBoostCost(0);
	Owner->SetBoostAmount(Owner->GetMaxBoostAmount());

	//FTimerHandle TempHandle;
	//GetWorld()->GetTimerManager().SetTimer(TempHandle, this, &UPowerupComponent::ClearPowerup, BoostPowerDuration, false);
}

void UPowerupComponent::ShieldPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}

	bShieldPowerupActive = true;
	ShieldPowerTime = ShieldPowerDuration;
	Owner->GetShieldMeshComponent()->SetVisibility(true);
	Owner->GetShieldMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Owner->GetShieldMeshComponent()->SetGenerateOverlapEvents(true);

	//FTimerHandle tempHandle;
	//GetWorld()->GetTimerManager().SetTimer(tempHandle, this, &UPowerupComponent::ClearPowerup, ShieldPowerDuration, false);


}

void UPowerupComponent::OverheatPowerup()
{
	if (Owner == nullptr)
	{
		return;
	}

	AMinigun* Minigun = Owner->GetMinigun();
	Minigun->PoweredUp = true;
	Minigun->SetOverheatValue(0); //kanske ska sätta overheat till 0
	OverheatPowerTime = OverheatPowerDuration;
	Owner->SetMinigunDamage(Owner->GetMinigunDefaultDamage() * 2);
	bOverHeatPowerupActive = true;
	//Aktivera progressbar för powered ammo ovanpå overheat bar
}

void UPowerupComponent::ClearPowerup(int PowerupId)
{
	if (Owner == nullptr)
	{
		return;
	}

	AMinigun* Minigun = Owner->GetMinigun();

	if (Minigun == nullptr)
	{
		return;
	}
	
	switch (PowerupId)
	{

	case 1:
		Owner->GetHealthComponent()->IsPoweredUp = false;
		bHealthPowerupActive = false;
		break;

	case 2:
		Owner->ResetBoostCost();
		bBoostPowerupActive = false;
		break;
	case 3:
		Minigun->PoweredUp = false;
		Owner->SetMinigunDamage(Owner->GetMinigunDefaultDamage());
		bOverHeatPowerupActive = false;
		break;

	case 4:

		bShieldPowerupActive = false;
		Owner->GetShieldMeshComponent()->SetVisibility(false);
		Owner->GetShieldMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Owner->GetShieldMeshComponent()->SetGenerateOverlapEvents(false);

		break;
	default:
		break;
	}
}

void UPowerupComponent::ResetPowerups()
{
	ClearPowerup(1);
	ClearPowerup(2);
	ClearPowerup(3);
	ClearPowerup(4);
	Owner->SetHeldPowerup(0);
}

float UPowerupComponent::GetPowerupPercentage(float PowerupFloat, float PowerupDuration)
{
	return FMath::Clamp(PowerupFloat / PowerupDuration, 0.f, 1.f);
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


	if (bOverHeatPowerupActive)
	{
		FMath::Clamp(OverheatPowerTime -= 1 * DeltaTime, 0, OverheatPowerDuration); 

		if (OverheatPowerTime <= 0)
		{
			ClearPowerup(3);
		}
		
	}

	if (bHealthPowerupActive)
	{
		FMath::Clamp(HealthPowerTime -= 1 * DeltaTime, 0, HealthPowerDuration); 

		/* if (HealthPowerTime <= 0)
		{
			ClearPowerup(1);
		} */
		
	}

	if (bShieldPowerupActive)
	{
		FMath::Clamp(ShieldPowerTime -= 1 * DeltaTime, 0, ShieldPowerDuration); 

		if (ShieldPowerTime <= 0)
		{
			ClearPowerup(4);
		}
		
	}

	if (bBoostPowerupActive)
	{
		FMath::Clamp(BoostPowerTime -= 1 * DeltaTime, 0, BoostPowerDuration); 

		if (BoostPowerTime <= 0)
		{
			ClearPowerup(2);
		}
		
	}
	

	// if (poweredup)
	// variable * 10 * deltatime

	// if (IsPoweredUp)
	//{
	//	RemoveHealth(RegenPerSecond * DeltaTime);
	//}

	// ...
}

