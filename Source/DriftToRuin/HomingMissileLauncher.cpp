// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

#include "BaseVehiclePawn.h"
#include "PlayerTurret.h"
#include "Minigun.h"

AHomingMissileLauncher::AHomingMissileLauncher()
{
	TargetingRange = 7000.f;
	AmmoCapacity = 4.f;
	AmmoAmount = AmmoCapacity;
}

void AHomingMissileLauncher::PullTrigger()
{
	Super::PullTrigger();
	
}

void AHomingMissileLauncher::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
	//SetActorScale3D(FVector(0.08f, 0.08f, 0.08f));
}

void AHomingMissileLauncher::FindTarget()
{
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;

	FVector CameraLocation;
	FRotator CameraRotation;
	OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TargetingRange);

	TArray<AActor*> ToIgnore;
	ToIgnore.Add(this);
	ToIgnore.Add(GetOwner());
	ToIgnore.Add(CarOwner->GetTurret());
	ToIgnore.Add(CarOwner->GetMinigun());

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(ToIgnore);
	//GetWorld()->LineTraceSingleByObjectType()
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}




