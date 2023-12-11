// Fill out your copyright notice in the Description page of Project Settings.


#include "MinigunProjectile.h"
#include "BaseVehiclePawn.h"
#include "Minigun.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

AMinigunProjectile::AMinigunProjectile() { }

void AMinigunProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	if(!ProjectileOwner) return;
	
	const auto OwnerInstigator = ProjectileOwner->GetInstigatorController();
	if(!OwnerInstigator) return;
	
	auto DamageTypeClass = UDamageType::StaticClass();
	Damage = ProjectileOwner->GetMinigunDamage();
	ABaseVehiclePawn* HitActor = Cast<ABaseVehiclePawn>(OtherActor);
	if(!HitActor) return;

	auto Minigun = ProjectileOwner->GetMinigun();
	if(!Minigun) { return; }

	if (OtherComp == HitActor->GetShieldMeshComponent() && OtherComp->GetOwner() != ProjectileOwner)
	{
		UE_LOG(LogTemp, Display, TEXT("hit shield"));
		RadialForceComponent->FireImpulse();
		Minigun->ProjectilePool->Return(this);
		//Destroy();
		return;
	}
	
	if (OtherActor && OtherActor != this && OtherActor != ProjectileOwner && !HitActor->GetIsDead())
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerInstigator, this, DamageTypeClass);
	}
	
	if (OtherActor != ProjectileOwner && OtherComp != ProjectileOwner->GetShieldMeshComponent())
	{
		RadialForceComponent->FireImpulse();
		Minigun->ProjectilePool->Return(this, [&]
		{
			ProjectileMovementComponent->StopSimulating(SweepResult);
		});
		//Destroy();
	}
}

/*Projectile callback function for collision*/
void AMinigunProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpusle, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpusle, Hit);
	if (auto Minigun = ProjectileOwner->GetMinigun())
	{
		Minigun->ProjectilePool->Return(this);
	}
	//Destroy();
}

void AMinigunProjectile::BeginPlay()
{
	Super::BeginPlay();
	InitializeAudio();
	
	ProjectileOwner = Cast<ABaseVehiclePawn>(GetOwner());
	ensureMsgf(ProjectileOwner,	TEXT("Failed to get owner calling BeginPlay for MinigunProjectile."));
	ProjectileOwner = Cast<ABaseVehiclePawn>(GetOwner());
	ensureMsgf(ProjectileOwner,	TEXT("Failed to get owner calling BeginPlay for MinigunProjectile."));
}

void AMinigunProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}
