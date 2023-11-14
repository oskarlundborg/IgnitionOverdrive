// Fill out your copyright notice in the Description page of Project Settings.


#include "MinigunProjectile.h"
#include "BaseVehiclePawn.h"
#include "Kismet/GameplayStatics.h"

AMinigunProjectile::AMinigunProjectile()
{
	
}

/*Projectile callback function for collision*/
void AMinigunProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpusle, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpusle, Hit);
	auto ProjectileOwner = GetOwner();
	if(!ProjectileOwner) return;
	auto OwnerBaseVehiclePawn = Cast<ABaseVehiclePawn>(ProjectileOwner);
	auto OwnerInstigator = ProjectileOwner->GetInstigatorController();
	if(!OwnerInstigator) return;
	auto DamageTypeClass = UDamageType::StaticClass();
	Damage = OwnerBaseVehiclePawn->GetDamage();
	

	if(OtherActor && OtherActor != this && OtherActor != ProjectileOwner) UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerInstigator, this, DamageTypeClass);
	Destroy();
}

void AMinigunProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMinigunProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}



