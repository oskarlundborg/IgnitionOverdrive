// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.h"
#include "Minigun.h"
#include "PlayerVehiclePawn.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHomingMissileLauncher::AHomingMissileLauncher()
{
	TargetingRange = 7000.f;
	ChargeCap = 3.f; 
	ChargeAmount = 0;
	ChargeTime = 2.f;
	ChargeBuildUpRate = 2.f;
	ChargeValue = 0.f;
	ChargeValueCap = 100.f;
	CooldownDuration = 0.f;
	CooldownOneCharge = 5.f;
	CooldownTwoCharges = 9.f;
	CooldownThreeCharges = 13.f;
	CloseTargetRange = 2500.f;
	MidTargetRange = 4000.f;
	CloseRangeMagnitude = 35000.f;
	MidRangeMagnitude = 30000.f;
	FarRangeMagnitude = 26000.f;
	TargetingOffset = 878.f;
	bIsCharging = false;
	bIsOnCooldown = false;
	bCanLockOn = false;
	CurrentTarget = nullptr;
	LastTarget = nullptr;
	bCanPlayLockOn = true;
	CanLockOnAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Can Lock On Audio Component"));
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
   
}

void AHomingMissileLauncher::InitializeOwnerVariables()
{
	CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	CarOwnerPlayer = Cast<APlayerVehiclePawn>(GetOwner());
}

/*  Called when input action is started.
 *  Has two use cases, it either starts missile charging or shoots missiles,
 *  based on if the player has charged at least 1 missile or not. */
void AHomingMissileLauncher::PullTrigger()
{
	Super::PullTrigger();
	if(ChargeAmount == 0 && !GetWorldTimerManager().IsTimerActive(ChargeHandle))
	{
		if(bIsOnCooldown) return;
		CurrentTarget = nullptr;
		LastTarget = nullptr;
		FindTarget();
		ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(CurrentTarget);
		if(TargetVenchi && !CheckTargetIsDead(TargetVenchi)) OnChargeFire();	
	} else
	{
		bIsCharging = false;
		ChargeValue = 0.f;
		if(GetWorldTimerManager().IsTimerActive(ChargeHandle)) GetWorld()->GetTimerManager().ClearTimer(ChargeHandle);
	
		if(!CurrentTarget || ChargeAmount == 0 || GetWorldTimerManager().IsTimerActive(FireTimer)) return;
		bIsOnCooldown = true;
		SetCooldownDuration();
		GetWorldTimerManager().SetTimer(CooldownTimer, this, &AHomingMissileLauncher::ResetCooldown, CooldownDuration, false, CooldownDuration);
		OnFire();
	}
}
			
void AHomingMissileLauncher::ReleaseTrigger()
{
	Super::ReleaseTrigger();

}

AActor* AHomingMissileLauncher::GetLastTarget() const
{
	return LastTarget;
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
	CooldownDuration = 0.f;
	GetWorldTimerManager().ClearTimer(CooldownTimer);
}

/*Sets a cooldown duration on missiles, based on the charged amount when the weapon is fired*/
void AHomingMissileLauncher::SetCooldownDuration()
{
	switch (ChargeAmount)
	{
	case 1: CooldownDuration = CooldownOneCharge; break;
	case 2: CooldownDuration = CooldownTwoCharges; break;
	case 3: CooldownDuration = CooldownThreeCharges; break;
	default: CooldownDuration = 0.f; break;
	}
}

/*Getter for cooldown time used in HUD widget*/
float AHomingMissileLauncher::GetCooldownTime()
{
	if(GetWorldTimerManager().IsTimerActive(CooldownTimer)) return FMath::Floor(GetWorldTimerManager().GetTimerRemaining(CooldownTimer));
	return FMath::Floor(CooldownDuration);
}

/*Getter used in HUD widget, checks if the weapon in on cooldown*/
bool AHomingMissileLauncher::GetIsOnCooldown()
{
	return bIsOnCooldown;
}

float AHomingMissileLauncher::GetChargeValue()
{
	return ChargeValue;
}

float AHomingMissileLauncher::GetChargeCapValue()
{
	return ChargeValueCap;
}

/*Getter used in HUD widget, checks if a player can lock on a target*/
bool AHomingMissileLauncher::GetCanLockOnTarget()
{
	return bCanLockOn;
}

float AHomingMissileLauncher::GetTargetRange()
{
	return TargetingRange;
}

float AHomingMissileLauncher::GetChargeValueCap()
{
	return ChargeCap;
}

FTimerHandle& AHomingMissileLauncher::GetFireTimer()
{
	return FireTimer;
}

void AHomingMissileLauncher::SetChargeAmount(const float NewChargeAmount)
{
	ChargeAmount = NewChargeAmount;
}

/*Sets the weapon on cooldown. Used for AIs*/
void AHomingMissileLauncher::SetAICooldown()
{
	bIsOnCooldown = true;
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &AHomingMissileLauncher::ResetCooldown, 10.0f, false, 10.0f);
}

/*Called when a player dies, disabling shooting and reseting class values to their defaults*/
void AHomingMissileLauncher::DisableShooting()
{
	CurrentTarget = nullptr;
	ChargeAmount = 0;
	bIsCharging = false;
	ChargeValue = 0.f;
	bCanLockOn = false;
	GetWorldTimerManager().ClearTimer(FireTimer);
	ResetCooldown();
	if(CarOwner->IsA(AEnemyVehiclePawn::StaticClass()))
	{
		AEnemyVehiclePawn* AIOwner = Cast<AEnemyVehiclePawn>(GetOwner());
		GetWorld()->GetTimerManager().ClearTimer(AIOwner->GetMissileTimerHandle());
	} else
	{
		GetWorld()->GetTimerManager().ClearTimer(ChargeHandle);
	}
}

/*Charges missiles by accumulating ChargeValue and increments the amount of charged missiles. Stops charging when the charge value cap is hit.*/
void AHomingMissileLauncher::ChargeFire()
{
	if(!CurrentTarget || ChargeAmount == ChargeCap)
	{
		GetWorldTimerManager().ClearTimer(ChargeHandle);
		ChargeValue = 0.f;
	}
	
	float ChargeAccumulation = ChargeValue + ChargeBuildUpRate;
	ChargeValue = FMath::Clamp(ChargeAccumulation, 0.f, ChargeValueCap);
	if(ChargeValue == ChargeValueCap)
	{
		ChargeAmount++;
		ChargeValue = 0.f;
	}
}

/*Called after input action, if FindTarget line trace has a hit actor. Calls ChargeFire on a looping timer, starting the missile charging process*/
void AHomingMissileLauncher::OnChargeFire()
{
	bIsCharging = true;
	GetWorldTimerManager().SetTimer(ChargeHandle, this, &AHomingMissileLauncher::ChargeFire, ChargeTime / 100, true);
}

/*Fires a missile by spawning it and assigning the current target as its homing target after a brief delay
 *A valid acceleration magnitude is set on the projectiles movement component too (See GetValidMagnitude function)*/
void AHomingMissileLauncher::Fire(AActor* Target)
{
	if(!GetOwner()) return;
	if(!Target)
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
		return;
	}
	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation = GetProjectileSpawnPoint()->GetComponentRotation();
	auto Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation);
	MissileFired(ChargeAmount);
	Projectile->SetOwner(GetOwner());
	ABaseVehiclePawn* CarTarget = Cast<ABaseVehiclePawn>(Target);
	if(CarTarget == nullptr) return;
	
	Projectile->GetProjectileMovementComponent()->HomingAccelerationMagnitude = GetValidMagnitude(Target);

	FTimerHandle TargetHandle;
	FTimerDelegate TargetDelegate;
	TargetDelegate.BindUFunction(this, FName("SetTarget"), Projectile, CarTarget);
	GetWorldTimerManager().SetTimer(TargetHandle, TargetDelegate, 0.4f, false);
	
	if(--ChargeAmount <= 0)
	{
		GetWorldTimerManager().ClearTimer(FireTimer);
		Target = nullptr;
		if(GetOwner()->IsA(AEnemyVehiclePawn::StaticClass()))
		{
			SetAICooldown();
		}
	}
}

/*Called after input action if the weapon has at least 1 charged missile.
 *Calls Fire on a timer based on the fire rate.*/
void AHomingMissileLauncher::OnFire()
{
	FTimerDelegate FireDelegate;
	FireDelegate.BindUFunction(this, FName("Fire"), CurrentTarget);
	GetWorldTimerManager().SetTimer(FireTimer, FireDelegate, 0.5f, true, 0.f);
}

/*Called when an AI receives a target and is ready to fire
 *Calls Fire on a timer based on the fire rate.*/
void AHomingMissileLauncher::OnFireAI(AActor* Target, int32 Charge)
{
	FTimerDelegate FireDelegate;
	ChargeAmount = Charge;
	LastTarget = Target;
	FireDelegate.BindUFunction(this, FName("Fire"), Target);
	GetWorldTimerManager().SetTimer(FireTimer, FireDelegate, 0.5f, true, 0.f);
}

/*Used for setting a projectiles homing target*/
void AHomingMissileLauncher::SetTarget(ABaseProjectile* Projectile, ABaseVehiclePawn* Target)
{
	Projectile->GetProjectileMovementComponent()->HomingTargetComponent = Target->GetHomingTargetPoint();
}

void AHomingMissileLauncher::PlayCanLockOnSound()
{
	if(!bCanPlayLockOn) return;
	CanLockOnAudioComponent->Play();
	bCanPlayLockOn = false;
	GetWorldTimerManager().SetTimer(LockOnSoundTimer, this, &AHomingMissileLauncher::ResetCanLockOnSoundCooldown, 0.8f, false, 0.8f);
}

void AHomingMissileLauncher::ResetCanLockOnSoundCooldown()
{
	bCanPlayLockOn = true;
}

/*Calculates a valid acceleration magnitude value to be set
 * on the projectile movement component based on the distance
 * to the current target player.*/
float AHomingMissileLauncher::GetValidMagnitude(AActor* Target)
{
	float HomingMagnitude;
	const float TargetDistance = GetOwner()->GetDistanceTo(Target);
	if(TargetDistance < CloseTargetRange)
	{
		HomingMagnitude = CloseRangeMagnitude;
	} else if (TargetDistance > CloseTargetRange && TargetDistance < MidTargetRange)
	{
		HomingMagnitude = MidRangeMagnitude;
	} else
	{
		HomingMagnitude = FarRangeMagnitude;
	}
	return HomingMagnitude;
}

/*Performs a series of checks when the weapon is charging
 * and resets to default class values if one of those checks is violated*/
void AHomingMissileLauncher::CheckTargetStatus()
{
	if(!CurrentTarget) return;
	if(CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;
	APlayerController* OwnerPlayerController = Cast<APlayerController>(OwnerController);
	if(OwnerPlayerController == nullptr) return;
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(CurrentTarget);
	if(!CheckTargetLineOfSight(OwnerController, CurrentTarget) || !CheckTargetInScreenBounds(OwnerPlayerController) || !CheckTargetInRange(CarOwner) || CheckTargetIsDead(TargetVenchi))
	{
		CurrentTarget = nullptr;
		ChargeAmount = 0;
		bIsCharging = false;
		ChargeValue = 0.f;
		bCanLockOn = false;
		GetWorldTimerManager().ClearTimer(FireTimer);
		//GetWorldTimerManager().ClearTimer(ChargeHandle);
		//GetWorldTimerManager().ClearTimer(FireTimer);
	}
}

/*Checks if a player has a line of sight to a target*/
bool AHomingMissileLauncher::CheckTargetLineOfSight(const AController* Controller, const AActor* Target) const
{
	return Controller->LineOfSightTo(Target, {CarOwner->GetCameraLocation().X, CarOwner->GetCameraLocation().Y, CarOwner->GetCameraLocation().Z + 300.f});
}

/*Checks if a target is in players screen bounds*/
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

/*Checks if a target is in range*/
bool AHomingMissileLauncher::CheckTargetInRange(const ABaseVehiclePawn* VehicleOwner) const
{
	float CurrentDistance = VehicleOwner->GetDistanceTo(CurrentTarget);
	if(CurrentDistance <= TargetingRange) return true;
	return false;
}

/*Checks if a target is dead*/
bool AHomingMissileLauncher::CheckTargetIsDead(ABaseVehiclePawn* TargetVenchi) const
{
	return TargetVenchi->GetIsDead();
}

/*Performs a line trace and sets the current target to trace hit result if the hit actor is an opponent vehicle*/
void AHomingMissileLauncher::FindTarget()
{
	if(CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;
	
	FHitResult HitResult;
	bool bHit = PerformTargetLockLineTrace(HitResult);
	
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		CurrentTarget = HitResult.GetActor();
		LastTarget = CurrentTarget;
	}
}

/*Constantly performs a line trace to check if a player can target lock an opponent*/
void AHomingMissileLauncher::CheckCanLockOn()
{
	if(CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;

	FHitResult HitResult;
    bool bHit = PerformTargetLockLineTrace(HitResult);
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(HitResult.GetActor());
	if(bHit && TargetVenchi && HitResult.GetActor()->ActorHasTag(FName("Targetable")) &&
		CheckTargetLineOfSight(OwnerController, HitResult.GetActor()) && !TargetVenchi->GetIsDead() && !bIsOnCooldown && !bIsCharging)
	{
		if(!bCanLockOn)
		{
			bCanLockOn = true;
			PlayCanLockOnSound();
		}
	} else
	{
		bCanLockOn = false;
	}
}

/*Performs a line trace in the direction that the player is aiming towards*/
bool AHomingMissileLauncher::PerformTargetLockLineTrace(FHitResult& HitResult)
{
	if(bIsOnCooldown || bIsCharging) return false;
	if(CarOwner == nullptr) return false;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return false;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector TraceStart = CameraLocation + (CameraRotation.Vector() * TargetingOffset);
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * (TargetingRange - TargetingOffset));
	
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(this);
	ToIgnore.Add(GetOwner());
	ToIgnore.Add(CarOwner->GetMinigun());
	
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(ToIgnore);
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_GameTraceChannel4, TraceParams);
	return bHit;
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckTargetStatus();
	CheckCanLockOn();
}




