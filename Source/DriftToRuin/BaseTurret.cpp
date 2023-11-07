// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTurret.h"

// Sets default values
ABaseTurret::ABaseTurret()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Turret Root"));
	SetRootComponent(TurretRoot);

	TurretMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Turret Mesh"));
	TurretMesh->SetupAttachment(TurretRoot);
}

// Called when the game starts or when spawned
void ABaseTurret::BeginPlay()
{
	Super::BeginPlay();
	
}

USkeletalMeshComponent* ABaseTurret::GetTurretMesh() const
{
	return TurretMesh;
}

// Called every frame
void ABaseTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

