// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"

#include "BaseVehiclePawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AHomingProjectile::AHomingProjectile()
{
	ProjectileMovementComponent->bIsHomingProjectile = true;
}

void AHomingProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpusle, const FHitResult& Hit)
{
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpusle, Hit);
	auto ProjectileOwner = GetOwner();
	if(!ProjectileOwner) return;
	auto OwnerBaseVehiclePawn = Cast<ABaseVehiclePawn>(ProjectileOwner);
	auto OwnerInstigator = ProjectileOwner->GetInstigatorController();
	if(!OwnerInstigator) return;
	auto DamageTypeClass = UDamageType::StaticClass();
	Damage = OwnerBaseVehiclePawn->GetHomingDamage();

	if(OtherActor && OtherActor != this && OtherActor != ProjectileOwner) UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerInstigator, this, DamageTypeClass);
	Destroy();
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AHomingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


