// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "EnemyVehiclePawn.h"
#include "PlayerTurret.h"
#include "Minigun.h"
#include "Camera/CameraComponent.h"
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
	CloseTargetRange = 2500.f;
	MidTargetRange = 4000.f;
	CloseRangeMagnitude = 35000.f;
	MidRangeMagnitude = 30000.f;
	FarRangeMagnitude = 26000.f;
	bIsCharging = false;
	bIsOnCooldown = false;
	bCanLockOn = false;
	CurrentTarget = nullptr;
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
   
}

void AHomingMissileLauncher::InitializeOwnerVariables()
{
	CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	/*if(CarOwner == nullptr) return;
	OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	OwnerPlayerController = Cast<APlayerController>(OwnerController);*/
}

void AHomingMissileLauncher::PullTrigger()
{
	Super::PullTrigger();
	if(bIsOnCooldown) return;
	//ChargeAmount = 0;
	CurrentTarget = nullptr;
	LastTarget = nullptr;
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
	default: CooldownDuration = 0.f; break;
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

void AHomingMissileLauncher::SetAICooldown()
{
	bIsOnCooldown = true;
	GetWorldTimerManager().SetTimer(CooldownTimer, this, &AHomingMissileLauncher::ResetCooldown, 10.0f, false, 10.0f);
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
	GetWorldTimerManager().SetTimer(TargetHandle, TargetDelegate, 0.45f, false);
	
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

void AHomingMissileLauncher::OnFire()
{
	FTimerDelegate FireDelegate;
	FireDelegate.BindUFunction(this, FName("Fire"), CurrentTarget);
	GetWorldTimerManager().SetTimer(FireTimer, FireDelegate, 0.5f, true, 0.f);
	//GetWorldTimerManager().SetTimer(FireTimer, this, &AHomingMissileLauncher::Fire, 0.5f, true, 0.f);
}

void AHomingMissileLauncher::OnFireAI(AActor* Target, int32 Charge)
{
	FTimerDelegate FireDelegate;
	ChargeAmount = Charge;
	FireDelegate.BindUFunction(this, FName("Fire"), Target);
	GetWorldTimerManager().SetTimer(FireTimer, FireDelegate, 0.5f, true, 0.f);
	//GetWorldTimerManager().SetTimer(FireTimer, this, &AHomingMissileLauncher::FireAI, 0.5f, true, 0.f);
}

void AHomingMissileLauncher::SetTarget(ABaseProjectile* Projectile, ABaseVehiclePawn* Target)
{
	Projectile->GetProjectileMovementComponent()->HomingTargetComponent = Target->GetHomingTargetPoint();
}

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

void AHomingMissileLauncher::CheckHoverTargetStatus()
{
	if(!HoverTarget || !bCanLockOn || !CarOwner) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(HoverTarget);
	if(!CheckTargetLineOfSight(OwnerController, HoverTarget) || CheckTargetIsDead(TargetVenchi))
	{
		//UE_LOG(LogTemp, Error, TEXT("DEAD"));
		bCanLockOn = false;
		HoverTarget = nullptr;
	}
}

bool AHomingMissileLauncher::CheckTargetLineOfSight(const AController* Controller, const AActor* Target) const
{
	//FVector OwnerEyes(GetOwner()->GetActorLocation().X, GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z + 1000.f);
	return Controller->LineOfSightTo(Target, CarOwner->GetCameraLocation());
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
	float CurrentDistance = VehicleOwner->GetDistanceTo(CurrentTarget);
	if(CurrentDistance <= TargetingRange) return true;
	return false;
}

bool AHomingMissileLauncher::CheckTargetIsDead(ABaseVehiclePawn* TargetVenchi) const
{
	return TargetVenchi->GetIsDead();
}

void AHomingMissileLauncher::FindTarget()
{
	if(CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;
	
	FHitResult HitResult;
	bool bHit = PerformTargetLockSweep(HitResult);
	
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")))
	{
		CurrentTarget = HitResult.GetActor();
		LastTarget = CurrentTarget;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName())
}

void AHomingMissileLauncher::CheckCanLockOn()
{
	if(!bCanStartSweep)
	{
		UE_LOG(LogTemp, Error, TEXT("NO SWEEPY SWEEP"))
		return;
	}
	if(CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return;
	
	FHitResult HitResult;
    bool bHit = PerformTargetLockSweep(HitResult);
	ABaseVehiclePawn* TargetVenchi = Cast<ABaseVehiclePawn>(HitResult.GetActor());
	UE_LOG(LogTemp, Error, TEXT("ISSWEEPIN"));
	if(bHit && HitResult.GetActor()->ActorHasTag(FName("Targetable")) && TargetVenchi && !TargetVenchi->GetIsDead() && !bIsOnCooldown && !bIsCharging)
	{
		bCanLockOn = true;
		//UE_LOG(LogTemp, Warning, TEXT("Can LOCK"));
	} else
	{
		bCanLockOn = false;
		//UE_LOG(LogTemp, Warning, TEXT("No"));
	}
}

void AHomingMissileLauncher::CheckTargetOverlapBegin(AActor* HoverActor)
{
	bCanStartSweep = true;
	//HoverTarget = HoverActor;
}

void AHomingMissileLauncher::CheckTargetOverlapEnd(AActor* HoverActor)
{
	bCanStartSweep = false;
	//HoverTarget = nullptr;
}

bool AHomingMissileLauncher::PerformTargetLockSweep(FHitResult& HitResult)
{
	if(CarOwner == nullptr) return false;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if(OwnerController == nullptr) return false;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TargetingRange);
	
	TArray<AActor*> ToIgnore;
	ToIgnore.Add(this);
	ToIgnore.Add(GetOwner());
	//ToIgnore.Add(CarOwner->GetTurret()); KOMENTERAT EFTER TURRET FLYTT
	ToIgnore.Add(CarOwner->GetMinigun());
	
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(ToIgnore);
	//FCollisionShape SweepSphere = FCollisionShape::MakeSphere(70.f);
	FCollisionShape SweepBox = FCollisionShape::MakeBox(FVector(60.f, 90.f ,60.f));
	
	bool bHit = GetWorld()->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity,ECC_Vehicle, SweepBox, TraceParams);
	//DrawDebugSphere(GetWorld(), TraceEnd, SweepSphere.GetSphereRadius(), 20, FColor::Green, true);
	//DrawDebugBox(GetWorld(), TraceEnd, SweepBox.GetExtent(), FColor::Blue, true);
	return bHit;
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckTargetStatus();
	//CheckHoverTargetStatus();
	CheckCanLockOn();
	//if(GetOwner() && CurrentTarget) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), GetOwner()->GetDistanceTo(CurrentTarget)));
	//UE_LOG(LogTemp, Warning, TEXT("%f"), GetCooldownTime());
	//UE_LOG(LogTemp, Warning, TEXT("%f"), ChargeValue/ChargeCap);
}




