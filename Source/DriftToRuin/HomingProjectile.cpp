// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"

AHomingProjectile::AHomingProjectile()
{
	ProjectileMovementComponent->bIsHomingProjectile = true;
}

void AHomingProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void AHomingProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


