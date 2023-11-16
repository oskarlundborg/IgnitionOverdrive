// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "PlayerTurret.h"
#include "Minigun.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHomingMissileLauncher::AHomingMissileLauncher()
{
	TargetingRange = 7000.f;
	AmmoCapacity = 3.f;
	ChargeAmount = 0;
	bIsCharging = false;

	CurrentTarget = nullptr;
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
	AmmoAmount = AmmoCapacity;
}

void AHomingMissileLauncher::PullTrigger()
{
	if(AmmoAmount == 0) return;
	//ChargeAmount = 0;
	CurrentTarget = nullptr; 
	Super::PullTrigger();
	FindTarget();
	if(CurrentTarget) OnChargeFire();
}

void AHomingMissileLauncher::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	bIsCharging = false;
	if(GetWorldTimerManager().IsTimerActive(ChargeHandle)) GetWorld()->GetTimerManager().ClearTimer(ChargeHandle);
	
	if(!CurrentTarget || ChargeAmount == 0) return;
	OnFire();
}

bool AHomingMissileLauncher::IsCharging()
{
	return bIsCharging;
}

int32 AHomingMissileLauncher::GetChargeAmount()
{
	return ChargeAmount;
}

void AHomingMissileLauncher::ChargeFire()
{
	if(++ChargeAmount == AmmoAmount) GetWorldTimerManager().ClearTimer(ChargeHandle);
}

void AHomingMissileLauncher::OnChargeFire()
{
	bIsCharging = true;
	GetWorldTimerManager().SetTimer(ChargeHandle, this, &AHomingMissileLauncher::ChargeFire, 2.f, true, 2.f);
}

void AHomingMissileLauncher::Fire()
{
	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation = GetProjectileSpawnPoint()->GetComponentRotation();
	auto Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation);
	Projectile->SetOwner(GetOwner());
	Projectile->GetProjectileMovementComponent()->HomingTargetComponent = CurrentTarget->GetRootComponent();
	AmmoAmount--;

	if(--ChargeAmount <= 0)
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
		CurrentTarget = nullptr;
	}
}

void AHomingMissileLauncher::OnFire()
{
	GetWorldTimerManager().SetTimer(FireTimer, this, &AHomingMissileLauncher::Fire, 0.5f, true, 0.f);
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

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(ToIgnore);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Vehicle, TraceParams);
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		CurrentTarget = HitResult.GetActor();
	}

	//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName())
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}




