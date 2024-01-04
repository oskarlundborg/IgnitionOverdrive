
#include "Minigun.h"

#include "AIController.h"
#include "BaseProjectile.h"
#include "BaseVehiclePawn.h"
#include "DrawDebugHelpers.h"
#include "EnemyVehiclePawn.h"
#include "PlayerTurret.h"
#include "HomingMissileLauncher.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerVehiclePawn.h"
#include "Kismet/KismetMathLibrary.h"
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
	OnPullTrigger();
}

/*Called when input action is released/completed*/
void AMinigun::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	bIsFiring = false;
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimer);
}

void AMinigun::InitializeOwnerVariables()
{
	CarOwner = Cast<APlayerVehiclePawn>(GetOwner());
}

/*Handles firing logic, spawning of projectiles and adjusting their rotation, as well as playing muzzle flash VFX*/
void AMinigun::Fire()
{
	bIsFiring = true;

	FVector SpawnLocation = GetProjectileSpawnPoint()->GetComponentLocation();
	FRotator ProjectileRotation;

	/*Choses aim adjust function based on if the owner is a player or an AI*/
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

/*Handles logic after input action is started. Calls Fire function on a timer based on the fire rate.*/
void AMinigun::OnPullTrigger()
{
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
		MinigunDisableAudio();
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

/*Updates overheat every tick. Builds up overheat if the weapon is firing, and cools it down if not*/
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
}

/*Adjusts projectile rotation on spawn to aim towards the direction that the crosshair is pointing to.*/
void AMinigun::AdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation)
{
	if (CarOwner == nullptr) return;
	AController* OwnerController = Cast<AController>(CarOwner->GetController());
	if (OwnerController == nullptr) return;
	
	FVector CameraLocation;
	FRotator CameraRotation;

	OwnerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
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
	
	FVector HitEndLocation;
	if (bHit)
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

/*Adjusts projectile rotation on spawn to aim towards the direction of the AIs target.*/
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
			}
			else
			{
				EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
				EnemyLocation += FVector(0, 0, 200);
			}
		}
	}
	
	float RandomSpreadY = FMath::RandRange(-50, 50);
	float RandomSpreadZ = FMath::RandRange(-50, 50);

	EnemyLocation += FVector(0.f, RandomSpreadY * 4, RandomSpreadZ * 4);
	ProjectileRotation = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, EnemyLocation);
}

void AMinigun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateOverheat();
}

bool AMinigun::GetIsOverheated()
{
	return bIsOverheated;
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

/*Called when a player dies, disabling shooting and reseting minigun values to their defaults*/
void AMinigun::DisableShooting()
{
	bIsOverheated = false;
	OverheatValue = 0.f;
	MinigunDisableAudio();
	ReleaseTrigger();
}
