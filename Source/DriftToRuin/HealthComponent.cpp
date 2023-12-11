// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "BaseVehiclePawn.h"
#include "PowerupComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	CurrentHealth = MaxHealth;
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamageTaken);
}

void UHealthComponent::DamageTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* Instigator, AActor* DamageCauser)
{
	if(Damage <= 0.f) return;
	CurrentHealth -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), CurrentHealth);

	if (CurrentHealth <= 0 )
	{
		OnVehicleDeath(DamageCauser->GetOwner());
	}
	
	//if(IsDead()) GetOwner()->Destroy();
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsPoweredUp)
	{
		AddHealth(RegenPerSecond * DeltaTime);
	}
	
	// ...
}

float UHealthComponent::GetHealth() const
{
	return CurrentHealth;
}

float UHealthComponent::GetDefaultMaxHealth() const
{
    return DefaultMaxHealth;
}

float UHealthComponent::GetHealthPercent() const
{
	return CurrentHealth / MaxHealth;
}

void UHealthComponent::SetHealth(float NewHealth)
{
	CurrentHealth = FMath::Min(MaxHealth, FMath::Max(0, NewHealth));
}

void UHealthComponent::AddHealth(float HealthAmount)
{
	SetHealth(GetHealth() + HealthAmount);
	UE_LOG(LogTemp, Display, TEXT("HEALING: %f" ), HealthAmount);
}

void UHealthComponent::ResetMaxHealth()
{
	MaxHealth = DefaultMaxHealth;
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
}

void UHealthComponent::OnVehicleDeath(AActor* DamageCauser)
{
	//Vad som ska hända vid vehicle death
	OnVehicleDeathDelegate.Broadcast(DamageCauser);

	//Respawnar via blueprint just nu (med denna delegate)

}

bool UHealthComponent::IsDead() const
{
	return CurrentHealth <= 0;
}

void UHealthComponent::RegenerateHealth()
{
	ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	GetWorld()->GetTimerManager().SetTimer(RegenerateHandle, this, &UHealthComponent::StopRegenerating, CarOwner->GetPowerupComponent()->HealthPowerDuration, false);
}

void UHealthComponent::StopRegenerating()
{
	ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	CarOwner->GetPowerupComponent()->ClearPowerup(1);
}
