// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"

#include "BaseVehiclePawn.h"
#include "HomingMissileLauncher.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

AHomingProjectile::AHomingProjectile()
{
	ProjectileMovementComponent->bIsHomingProjectile = true;
	DestructionTime = 0.8f;
}

void AHomingProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	auto ProjectileOwner = GetOwner();
	if(!ProjectileOwner) return;
	auto OwnerBaseVehiclePawn = Cast<ABaseVehiclePawn>(ProjectileOwner);
	auto OwnerInstigator = ProjectileOwner->GetInstigatorController();
	if(!OwnerInstigator) return;
	auto DamageTypeClass = UDamageType::StaticClass();
	Damage = OwnerBaseVehiclePawn->GetHomingDamage();
	ABaseVehiclePawn* HitActor = Cast<ABaseVehiclePawn>(OtherActor);
	if(!HitActor) return;
	
	if (OtherComp == HitActor->GetShieldMeshComponent() && OtherComp->GetOwner() != ProjectileOwner)
	{
		OnDestroy();
		ProjectileMesh->SetGenerateOverlapEvents(false);
		return;
	}
	
	if(OtherActor && OtherActor != this && OtherActor != ProjectileOwner && !HitActor->GetIsDead()
		&& OtherComp != HitActor->GetShieldMeshComponent()) UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerInstigator, this, DamageTypeClass);
	
	if(OtherActor != ProjectileOwner && OtherComp != OwnerBaseVehiclePawn->GetShieldMeshComponent())
	{
		OnDestroy();
	}
}

void AHomingProjectile::CheckIfTargetDied()
{
	if(GetProjectileMovementComponent()->HomingTargetComponent == nullptr) return;
	auto OwnerPawn = Cast<ABaseVehiclePawn>(GetOwner());
	if(!OwnerPawn) return;
	auto Target = Cast<ABaseVehiclePawn>(OwnerPawn->GetHomingLauncher()->GetLastTarget());
	if(!Target) return;
	//UE_LOG(LogTemp, Warning, TEXT("Dead"));
	if(Target->GetIsDead())
	{
		GetProjectileMovementComponent()->HomingTargetComponent = nullptr;
		//GetProjectileMovementComponent()->ProjectileGravityScale(1.0);
		//Destroy();
		//UE_LOG(LogTemp, Warning, TEXT("Dead"));
	}
}

void AHomingProjectile::OnDestroy()
{
	RadialForceComponent->FireImpulse();
	ProjectileVfxNiagaraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	DestructionDelegate.BindLambda([this] {if(this->IsValidLowLevel()) Destroy();});
	GetWorldTimerManager().SetTimer(DestroyTimer, DestructionDelegate, DestructionTime, false);
	SetActorEnableCollision(false);
}

void AHomingProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpusle, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpusle, Hit);
	Destroy();
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AHomingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckIfTargetDied();
}


