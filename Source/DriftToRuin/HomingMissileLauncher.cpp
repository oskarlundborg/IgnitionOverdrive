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
	ChargeCap = 3.f; 
	ChargeAmount = 0;
	ChargeTime = 2.f;
	CooldownDuration = 10.f;
	bIsCharging = false;
	bIsOnCooldown = false;
	CurrentTarget = nullptr;
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
	//AmmoAmount = ChargeCap; // here
}

void AHomingMissileLauncher::PullTrigger()
{
	if(bIsOnCooldown) return;
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
	
	if(!CurrentTarget || ChargeAmount == 0 || GetWorldTimerManager().IsTimerActive(FireTimer)) return;
	bIsOnCooldown = true;
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &AHomingMissileLauncher::ResetCooldown, CooldownDuration, false, CooldownDuration);
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

void AHomingMissileLauncher::ResetCooldown()
{
	bIsOnCooldown = false;
	GetWorldTimerManager().ClearTimer(CooldownTimer);
}

void AHomingMissileLauncher::ResetAmmo()
{
	//AmmoAmount = ChargeCap; // here
}

void AHomingMissileLauncher::SetAmmo(int32 Amount)
{
	//if(Amount > ChargeCap) return; // here
	//AmmoAmount = Amount;
}

int32 AHomingMissileLauncher::GetAmmo()
{
	//return AmmoAmount; // here
	return 0;
}

void AHomingMissileLauncher::ChargeFire()
{
	if(!CurrentTarget || ++ChargeAmount == ChargeCap) GetWorldTimerManager().ClearTimer(ChargeHandle);
}

void AHomingMissileLauncher::OnChargeFire()
{
	bIsCharging = true;
	GetWorldTimerManager().SetTimer(ChargeHandle, this, &AHomingMissileLauncher::ChargeFire, ChargeTime, true, ChargeTime);
}

void AHomingMissileLauncher::Fire()
{
	if(!GetOwner()) return;
	if(!CurrentTarget)
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
		return;
	}
	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation = GetProjectileSpawnPoint()->GetComponentRotation();
	auto Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation);
	Projectile->SetOwner(GetOwner());
	Projectile->GetProjectileMovementComponent()->HomingTargetComponent = CurrentTarget->GetRootComponent();

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

void AHomingMissileLauncher::CheckTargetVisibility()
{
	if(!CurrentTarget) return;
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	const AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	const APlayerController* OwnerPlayerController = Cast<APlayerController>(OwnerController);
	if(OwnerPlayerController == nullptr) return;

	if(!CheckTargetLineOfSight(OwnerController) || !CheckTargetInScreenBounds(OwnerPlayerController) || !CheckTargetInRange(CarOwner))
	{
		CurrentTarget = nullptr;
		ChargeAmount = 0;
		bIsCharging = false;
		//GetWorldTimerManager().ClearTimer(ChargeHandle);
		//GetWorldTimerManager().ClearTimer(FireTimer);
	}
}

bool AHomingMissileLauncher::CheckTargetLineOfSight(const AController* Controller) const
{
	return Controller->LineOfSightTo(CurrentTarget);
}

bool AHomingMissileLauncher::CheckTargetInScreenBounds(const APlayerController* PlayerController) const
{
	int32 ViewportSizeX;
	int32 ViewportSizeY;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	FVector TargetLocation = CurrentTarget->GetActorLocation();
	FVector2D ScreenLocation;
	bool bIsOnScreen = PlayerController->ProjectWorldLocationToScreen(TargetLocation, ScreenLocation);

	if(bIsOnScreen && ScreenLocation.X >= 0 && ScreenLocation.X <= ViewportSizeX && ScreenLocation.Y >= 0 && ScreenLocation.Y <= ViewportSizeY) return true;
	return false;
}

bool AHomingMissileLauncher::CheckTargetInRange(const ABaseVehiclePawn* VehicleOwner) const
{
	float CurrentDistance = Owner->GetDistanceTo(CurrentTarget);
	if(CurrentDistance <= TargetingRange) return true;
	return false;
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
	FCollisionShape SweepSphere = FCollisionShape::MakeSphere(60.f);
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,ECC_Vehicle, SweepSphere, TraceParams);
	//DrawDebugSphere(GetWorld(), TraceEnd, SweepSphere.GetSphereRadius(), 30, FColor::Green, true);
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		CurrentTarget = HitResult.GetActor();
	}
	//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName())
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckTargetVisibility();
}




