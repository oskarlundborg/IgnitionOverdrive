// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerTurret.h"

#include "BaseVehiclePawn.h"
#include "PlayerVehiclePawn.h"
#include "Camera/CameraComponent.h"

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
	/*const APlayerVehiclePawn* CarOwner = Cast<APlayerVehiclePawn>(GetOwner());
	if(!CarOwner) return;
	const AController* OwnerController = CarOwner->GetController();
	if(!OwnerController) return;
	const FRotator ControllerRotation = OwnerController->GetControlRotation();
	const FRotator YawRotation(0.f, ControllerRotation.Yaw, 0.f);
	//const FRotator BaseRotation = GetTurretMesh()->GetRelativeRotation();
	const FRotator TurretRotation = GetTurretMesh()->GetComponentRotation();
	const FRotator NewTurretRotation = FMath::RInterpTo(TurretRotation, YawRotation, GetWorld()->GetDeltaSeconds(), 30);
	//GetTurretMesh()->SetRelativeRotation(NewBaseRotation);
	GetTurretMesh()->SetWorldRotation(NewTurretRotation);*/
}

void APlayerTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateTurretRotation();
}
