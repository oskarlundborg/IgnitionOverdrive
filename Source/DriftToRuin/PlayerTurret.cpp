// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerTurret.h"

#include "BaseVehiclePawn.h"

APlayerTurret::APlayerTurret()
{
	
}

void APlayerTurret::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerTurret::UpdateTurretRotation()
{
	Super::UpdateTurretRotation();
	const ABaseVehiclePawn* CarOwner = Cast<ABaseVehiclePawn>(GetOwner());
	if(!CarOwner) return;
	const AController* OwnerController = CarOwner->GetController();
	if(!OwnerController) return;

	const FRotator ControllerRotation = OwnerController->GetControlRotation();
	const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);
	const FRotator BaseRotation = GetTurretMesh()->GetRelativeRotation();
	const FRotator NewBaseRotation = FMath::RInterpTo(BaseRotation, YawRotation, GetWorld()->GetDeltaSeconds(), 30);
	GetTurretMesh()->SetRelativeRotation(NewBaseRotation);
}

void APlayerTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateTurretRotation();
}


