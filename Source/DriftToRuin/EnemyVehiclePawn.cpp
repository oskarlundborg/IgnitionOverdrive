// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyVehiclePawn.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "PlayerTurret.h"

AEnemyVehiclePawn::AEnemyVehiclePawn()
{
	
}

void AEnemyVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretSocket"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MinigunSocket"));
	Minigun->SetOwner(this);

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("HomingSocket"));
	HomingLauncher->SetOwner(this);
}

void AEnemyVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
