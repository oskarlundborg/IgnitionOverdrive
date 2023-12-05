// Fill out your copyright notice in the Description page of Project Settings.


#include "MinigunProjectile.h"
#include "BaseVehiclePawn.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"

AMinigunProjectile::AMinigunProjectile()
{
	
}

void AMinigunProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	auto ProjectileOwner = GetOwner();
	if(!ProjectileOwner) return;
	auto OwnerBaseVehiclePawn = Cast<ABaseVehiclePawn>(ProjectileOwner);
	auto OwnerInstigator = ProjectileOwner->GetInstigatorController();
	if(!OwnerInstigator) return;
	auto DamageTypeClass = UDamageType::StaticClass();
	Damage = OwnerBaseVehiclePawn->GetMinigunDamage();
	ABaseVehiclePawn* HitActor = Cast<ABaseVehiclePawn>(OtherActor);
	if(!HitActor) return;

	if (OtherComp == HitActor->GetShieldMeshComponent() && OtherComp->GetOwner() != ProjectileOwner)
	{
		UE_LOG(LogTemp, Display, TEXT("hit shield"));
		RadialForceComponent->FireImpulse();
		Destroy();
		return;
	}
	
	if(OtherActor && OtherActor != this && OtherActor != ProjectileOwner && !HitActor->GetIsDead()) UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerInstigator, this, DamageTypeClass);
	if(OtherActor != ProjectileOwner && OtherComp != OwnerBaseVehiclePawn->GetShieldMeshComponent())
	{
		RadialForceComponent->FireImpulse();
		Destroy();
	}
}

/*Projectile callback function for collision*/
void AMinigunProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpusle, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpusle, Hit);
	Destroy();
}

void AMinigunProjectile::BeginPlay()
{
	Super::BeginPlay();
	InitializeAudio();
}

void AMinigunProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}



