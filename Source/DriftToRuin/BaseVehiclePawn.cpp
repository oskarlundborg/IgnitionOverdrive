// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseVehiclePawn.h"
#include "HealthComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABaseVehiclePawn::ABaseVehiclePawn()
{
    //Get Vehicle Movement Component when its needed
	

	//Creates Health Component and sets it max health value
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetMaxHealth(MaxHealth);
	
	//Camera & SpringArm may not be necessary in AI, move to player subclass if decided.
	
	//Create SpringArm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 500.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;

	//Create Camera Component
	//(Camera panning constraints will be determined using Camera Manager)
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->FieldOfView = 90.0f;
}

void ABaseVehiclePawn::BeginPlay()
{
	
}

void ABaseVehiclePawn::Tick(float DeltaSeconds)
{
	
}



