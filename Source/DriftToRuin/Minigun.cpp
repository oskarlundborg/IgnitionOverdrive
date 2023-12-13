// Fill out your copyright notice in the Description page of Project Settings.


#include "Minigun.h"

#include "AIController.h"
#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "DrawDebugHelpers.h"
#include "EnemyVehiclePawn.h"
#include "PlayerTurret.h"
#include "HomingMissileLauncher.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerVehiclePawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "PowerupComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
}

/*Called when input action is started*/
void AMinigun::PullTrigger()
{
	if (bIsOverheated) return;
	Super::PullTrigger();
	//bIsFiring = true;
	//OverheatValue += 4.5f;
	OnPullTrigger();
}

/*Called when input action is released/completed*/
void AMinigun::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimer);
}

bool AMinigun::GetIsOverheated()
{
	return bIsOverheated;
}

void AMinigun::InitializeOwnerVariables()
{
	CarOwner = Cast<APlayerVehiclePawn>(GetOwner());
	//if (CarOwner == nullptr) return;
	//OwnerController = CarOwner->GetController();
}

/*Handles firing logic, meaning spawning projectiles*/
void AMinigun::Fire()
{
	//MuzzleFlashNiagaraComponent->Activate();
	bIsFiring = true;

	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation;

	//if sats, välj mellan AI adjust eller vanlig adjust.

	bool IsAI = false;
	if (GetOwner()->IsA(AEnemyVehiclePawn::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("AI adjust projectile true"));
		AIAdjustProjectileAimToCrosshair(SpawnLocation, ProjectileRotation);
		IsAI = true;
	}
	else
	{
		AdjustProjectileAimToCrosshair(SpawnLocation, ProjectileRotation);
	}

	auto Projectile = GetWorld()->SpawnActor<ABaseProjectile>(ProjectileClass, SpawnLocation, ProjectileRotation);
	if (IsAI)
	{
		Projectile->GetProjectileMovementComponent()->ProjectileGravityScale = 0.0f;
	}

	ProjectileSpawned(Projectile);
	UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFlashNiagaraSystem, GetWeaponMesh(), FName("MuzzleFlashSocket"),
	                                             GetWeaponMesh()->GetSocketLocation(FName("MuzzleFlashSocket")),
	                                             GetWeaponMesh()->GetSocketRotation(FName("MuzzleFlashSocket")),
	                                             EAttachLocation::KeepWorldPosition, true);
	Projectile->SetOwner(GetOwner());
}

/*Handles logic after input action is started*/
void AMinigun::OnPullTrigger()
{
	//if(!bIsFiring) return;
	//Fire();
	GetWorld()->GetTimerManager().SetTimer(FireRateTimer, this, &AMinigun::Fire, FireRate, true, 0.8f);
}

/*Builds up overheat while the weapon is firing*/
void AMinigun::BuildUpOverheat()
{
	float OverheatAccumulation = OverheatValue + (OverheatBuildUpRate * GetWorld()->DeltaTimeSeconds);
	OverheatValue = FMath::Clamp(OverheatAccumulation, 0.f, OverheatMax);
	if (OverheatValue == OverheatMax)
	{
		bIsOverheated = true;
		MinigunFullOverheat();
		ReleaseTrigger();
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::OverheatCooldown, OverheatCooldownDuration,
		                                       false);
	}
}

/*Cools down the weapon when it is not firing*/
void AMinigun::CoolDownWeapon()
{
	if (bIsFiring || bIsOverheated) return;
	float OverheatCoolDown = OverheatValue - (OverheatCoolDownRate * GetWorld()->DeltaTimeSeconds);
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
	if (bIsFiring && !PoweredUp)
	{
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::BuildUpOverheat, 0.1f, false);
	}
	else if (!bIsFiring && OverheatValue != 0.f && !bIsOverheated && !PoweredUp)
	{
		FTimerHandle THandle;
		GetWorld()->GetTimerManager().SetTimer(THandle, this, &AMinigun::CoolDownWeapon, 0.15f, false);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, FString::Printf(TEXT("Overheat Value: %f"), OverheatValue));
}

/*Adjusts projectile rotation on spawn to aim towards the crosshair*/
void AMinigun::AdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation)
{
	if (CarOwner == nullptr) return;
	//if (OwnerController == nullptr) return;
	AController* Bajs = Cast<AController>(CarOwner->GetController());

	FVector CameraLocation;
	FRotator CameraRotation;

	Bajs->GetPlayerViewPoint(CameraLocation, CameraRotation);
	FVector OffsetVector = CarOwner->GetTurret()->GetActorLocation() - CameraLocation;
	float OffsetLenght = OffsetVector.Length();
	FVector TraceStart = CameraLocation + (CameraRotation.Vector() * OffsetLenght);
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * TraceDistance);

	ToIgnore.AddUnique(GetOwner());
	ToIgnore.AddUnique(CarOwner->GetTurret());
	ToIgnore.AddUnique(CarOwner->GetHomingLauncher());
	ToIgnore.AddUnique(this);

	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActors(ToIgnore);

	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, true);
	FVector HitEndLocation;
	if (bHit)
	{
		HitEndLocation = HitResult.ImpactPoint;
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("HIT %s"), *HitResult.GetActor()->GetName()));
	}
	else
	{
		HitEndLocation = HitResult.TraceEnd;
	}

	float RandomSpreadY = FMath::RandRange(ProjSpreadMinY, ProjSpreadMaxY);
	float RandomSpreadZ = FMath::RandRange(ProjSpreadMinZ, ProjSpreadMaxZ);

	HitEndLocation += FVector(0.f, RandomSpreadY, RandomSpreadZ);
	//FVector SpawnSpread = SpawnLocation + FVector(0.f, RandomSpawnSpreadY, RandomSpawnSpreadZ);

	ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, HitEndLocation);
	//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, true);
}

//projectile transform adjust for AI firing
void AMinigun::AIAdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation)
{
	const AEnemyVehiclePawn* AIOwner = Cast<AEnemyVehiclePawn>(GetOwner());
	if (AIOwner == nullptr) return;
	FVector EnemyLocation;
	AAIController* AIController = AIOwner->GetController<AAIController>();
	if (AIController != nullptr)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
		if (BlackboardComp != nullptr)
		{
			UObject* EnemyObject = BlackboardComp->GetValueAsObject("Enemy");
			ABaseVehiclePawn* EnemyVehicle = Cast<ABaseVehiclePawn>(EnemyObject);
			if (EnemyVehicle != nullptr)
			{
				EnemyLocation = EnemyVehicle->GetHomingTargetPoint()->GetComponentLocation();
				UE_LOG(LogTemp, Warning, TEXT("Using homin target point"));
			}
			else
			{
				EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
				EnemyLocation += FVector(0, 0, 200);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BlackboardComponent is nullptr"));
		}
	}

	//göras om till AI spread ifall vi ska ha nivåer 
	float RandomSpreadY = FMath::RandRange(-50, 50);
	float RandomSpreadZ = FMath::RandRange(-50, 50);

	EnemyLocation += FVector(0.f, RandomSpreadY, RandomSpreadZ);
	ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, EnemyLocation);
}

void AMinigun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateOverheat();
}

float AMinigun::GetOverheatValue() const
{
	return OverheatValue;
}

float AMinigun::GetOverheatMaxValue() const
{
	return OverheatMax;
}

float AMinigun::GetPowerAmmoPercent()
{
	return PowerAmmo / 100;
}

bool AMinigun::GetIsPoweredUp()
{
	return PoweredUp;
}

bool AMinigun::GetIsFiring()
{
	return bIsFiring;
}
