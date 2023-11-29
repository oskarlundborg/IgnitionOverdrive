// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "PlayerTurret.h"
#include "Minigun.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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
	MagnitudeChangeRange = 2500.f;
	CloseRangeMagnitude = 35000.f;
	FarRangeMagnitude = 26000.f;
	bIsCharging = false;
	bIsOnCooldown = false;
	bCanLockOn = false;
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
	LastTarget = nullptr;
	Super::PullTrigger();
	FindTarget();
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(CurrentTarget);
	if(TargetVenchi && !CheckTargetIsDead(TargetVenchi)) OnChargeFire();
}
			
void AHomingMissileLauncher::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	bIsCharging = false;
	ChargeValue = 0.f;
	if(GetWorldTimerManager().IsTimerActive(ChargeHandle)) GetWorld()->GetTimerManager().ClearTimer(ChargeHandle);
	
	if(!CurrentTarget || ChargeAmount == 0 || GetWorldTimerManager().IsTimerActive(FireTimer)) return;
	bIsOnCooldown = true;
	SetCooldownDuration();
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &AHomingMissileLauncher::ResetCooldown, CooldownDuration, false, CooldownDuration);
	OnFire();
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

void AHomingMissileLauncher::SetCooldownDuration()
{
	switch (ChargeAmount)
	{
	case 1: CooldownDuration = CooldownOneCharge; break;
	case 2: CooldownDuration = CooldownTwoCharges; break;
	case 3: CooldownDuration = CooldownThreeCharges; break;
	default: CooldownDuration = 0.f;
	}
}

float AHomingMissileLauncher::GetCooldownTime()
{
	if(GetWorldTimerManager().IsTimerActive(CooldownTimer)) return FMath::Floor(GetWorldTimerManager().GetTimerRemaining(CooldownTimer));
	return FMath::Floor(CooldownDuration);
}

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
	//if(ChargeValue == ChargeCap) GetWorldTimerManager().ClearTimer(ChargeHandle); //ChargeValue = 0.f;
	//if(!CurrentTarget || ++ChargeAmount == ChargeCap) GetWorldTimerManager().ClearTimer(ChargeHandle);
}
	
void AHomingMissileLauncher::OnChargeFire()
{
	bIsCharging = true;
	GetWorldTimerManager().SetTimer(ChargeHandle, this, &AHomingMissileLauncher::ChargeFire, ChargeTime / 100, true);
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
	MissileFired(ChargeAmount);
	Projectile->SetOwner(GetOwner());
	const ABaseVehiclePawn* CarTarget = Cast<ABaseVehiclePawn>(CurrentTarget);
	if(CarTarget == nullptr) return;
	Projectile->GetProjectileMovementComponent()->HomingTargetComponent = CarTarget->GetHomingTargetPoint();

	float HoAccMa = FarRangeMagnitude;
	if(GetOwner()->GetDistanceTo(CurrentTarget) < MagnitudeChangeRange)
	{
		HoAccMa = CloseRangeMagnitude;
	}
	Projectile->GetProjectileMovementComponent()->HomingAccelerationMagnitude = HoAccMa;
	
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
	
void AHomingMissileLauncher::CheckTargetStatus()
{
	if(!CurrentTarget) return;
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(CurrentTarget);
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	const AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	const APlayerController* OwnerPlayerController = Cast<APlayerController>(OwnerController);
	if(OwnerPlayerController == nullptr) return;
	
	if(!CheckTargetLineOfSight(OwnerController) || !CheckTargetInScreenBounds(OwnerPlayerController) || !CheckTargetInRange(CarOwner) || CheckTargetIsDead(TargetVenchi))
	{
		CurrentTarget = nullptr;
		ChargeAmount = 0;
		bIsCharging = false;
		ChargeValue = 0.f;
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

bool AHomingMissileLauncher::CheckTargetIsDead(ABaseVehiclePawn* TargetVenchi) const
{
	return TargetVenchi->GetIsDead();
}

void AHomingMissileLauncher::FindTarget()
{
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	
	FHitResult HitResult;
	bool bHit = PerformTargetLockSweep(HitResult);
	//DrawDebugSphere(GetWorld(), TraceEnd, SweepSphere.GetSphereRadius(), 30, FColor::Green, true);
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		CurrentTarget = HitResult.GetActor();
		LastTarget = CurrentTarget;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName())
}

void AHomingMissileLauncher::CheckCanLockOn()
{
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	
	FHitResult HitResult;
    bool bHit = PerformTargetLockSweep(HitResult);
	//DrawDebugSphere(GetWorld(), TraceEnd, SweepSphere.GetSphereRadius(), 30, FColor::Green, true);
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		bCanLockOn = true;
		UE_LOG(LogTemp, Warning, TEXT("Can LOCK"));
	} else
	{
		bCanLockOn = false;
		UE_LOG(LogTemp, Warning, TEXT("No"));
	}
}

bool AHomingMissileLauncher::PerformTargetLockSweep(FHitResult& HitResult)
{
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return false;
	AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return false;
	
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
	FCollisionShape SweepSphere = FCollisionShape::MakeSphere(70.f);
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,ECC_Vehicle, SweepSphere, TraceParams);
	return bHit;
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckTargetStatus();
	CheckCanLockOn();
	//if(GetOwner() && CurrentTarget) UE_LOG(LogTemp, Warning, TEXT("%f"), GetOwner()->GetDistanceTo(CurrentTarget));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), GetCooldownTime());
	//UE_LOG(LogTemp, Warning, TEXT("%f"), ChargeValue/ChargeCap);
}




