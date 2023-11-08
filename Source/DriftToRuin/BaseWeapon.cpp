// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Root"));
	SetRootComponent(WeaponRoot);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Projectile Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(WeaponMesh);
}

USkeletalMeshComponent* ABaseWeapon::GetWeaponMesh() const
{
	return WeaponMesh;
}

USceneComponent* ABaseWeapon::GetProjectileSpawnPoint() const
{
	return ProjectileSpawnPoint;
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

