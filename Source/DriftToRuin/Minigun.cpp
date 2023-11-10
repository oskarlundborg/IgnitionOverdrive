// Fill out your copyright notice in the Description page of Project Settings.


#include "Minigun.h"

#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AMinigun::AMinigun()
{
	/*Spread range variables*/
	ProjSpreadMinY = -30.f;
	ProjSpreadMaxY = 30.f;
	ProjSpreadMinZ = -30.f;
	ProjSpreadMaxZ = 30.f;
	/*Firing and overheat variables*/
	FireRate = 0.5f;
	TraceDistance = 10000.f;
	bIsFiring = false;
	bIsOverheated = false;
	OverheatValue = 0.f;
	OverheatMax = 100.f;
	OverheatBuildUpRate = 0.5f;
	OverheatCoolDownRate = 3.f;
	OverheatCooldownDuration = 2.5f;
}

void AMinigun::BeginPlay()
{
	Super::BeginPlay();
	//SetActorScale3D(FVector(0.08f, 0.08f, 0.08f));
}

/*Called when input action is started*/
void AMinigun::PullTrigger()
{
	if(bIsOverheated) return;
	Super::PullTrigger();
	bIsFiring = true;
	OnPullTrigger();
}

/*Called when input action is released/completed*/
void AMinigun::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimer);
}

/*Handles firing logic, meaning spawning projectiles*/
void AMinigun::Fire()
{
	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation;
	AdjustProjectileAimToCrosshair(SpawnLocation, ProjectileRotation);
	GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation);
}

/*Handles logic after input action is started*/
void AMinigun::OnPullTrigger()
{
	if(!bIsFiring) return;
	Fire();
	GetWorld()->GetTimerManager().SetTimer(FireRateTimer, this, &AMinigun::OnPullTrigger, FireRate, true);
}

/*Builds up overheat while the weapon is firing*/
void AMinigun::BuildUpOverheat()
{
	float OverheatAccumulation = OverheatValue + OverheatBuildUpRate;
	OverheatValue = FMath::Clamp(OverheatAccumulation, 0.f, OverheatMax);
	if(OverheatValue == OverheatMax)
	{
		bIsOverheated = true;
		ReleaseTrigger();
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::OverheatCooldown, OverheatCooldownDuration, false);
	}
}

/*Cools down the weapon when it is not firing*/
void AMinigun::CoolDownWeapon()
{
	float OverheatCoolDown = OverheatValue - OverheatCoolDownRate;
	OverheatValue = FMath::Clamp(OverheatCoolDown, 0.f, OverheatMax);
}

/*Called after Overheat cooldown timer*/
void AMinigun::OverheatCooldown()
{
	bIsOverheated = false;
}

/*Updates overheat every tick if the gun is firing or not firing and is not at 0*/
void AMinigun::UpdateOverheat()
{
	if(bIsFiring)
	{
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::BuildUpOverheat, 0.1f, false);
	} else if(!bIsFiring && OverheatValue != 0.f && !bIsOverheated)
	{
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::CoolDownWeapon, 0.04f, false);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Overheat Value: %f"), OverheatValue));
}

/*Adjusts projectile rotation on spawn to aim towards the crosshair*/
void AMinigun::AdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation)
{
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(CarOwner == nullptr) return;
	AController* OwnerController = CarOwner->GetController();
	if(OwnerController == nullptr) return;
	
	FVector CameraLocation;
	FRotator CameraRotation;
	FHitResult HitResult;
	
	OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TraceDistance);
	
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this); //CHANGE TO AN ARRAY OF ACTORS TO IGNORE!
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	FVector HitEndLocation; 
	if(bHit)
	{
		HitEndLocation = HitResult.ImpactPoint;
	}
	else
	{
		HitEndLocation = HitResult.TraceEnd;
	}

	float RandomSpreadY = FMath::RandRange(ProjSpreadMinY, ProjSpreadMaxY);
	float RandomSpreadZ = FMath::RandRange(ProjSpreadMinZ, ProjSpreadMaxZ);

	HitEndLocation += FVector(0.f, RandomSpreadY, RandomSpreadZ);
	
	ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, HitEndLocation);
}

void AMinigun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateOverheat();
}



