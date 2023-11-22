// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "NiagaraComponent.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Root"));
	SetRootComponent(WeaponRoot);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetupAttachment(WeaponRoot);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Projectile Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(WeaponMesh);

	MuzzleFlashNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Muzzle Flash Component"));
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

	if(MuzzleFlashNiagaraComponent)
	{
		//MuzzleFlashNiagaraComponent->AttachToComponent(WeaponMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT("MuzzleFlashSocket"));
		MuzzleFlashNiagaraComponent->SetAsset(MuzzleFlashNiagaraSystem);
		MuzzleFlashNiagaraComponent->Deactivate();
	}
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
