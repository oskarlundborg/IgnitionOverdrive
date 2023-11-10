// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseVehiclePawn.h"
#include "HealthComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

ABaseVehiclePawn::ABaseVehiclePawn()
{
    //Get Vehicle Movement Component when its needed
	UChaosWheeledVehicleMovementComponent* VehicleMovementComp = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	//Engine value defaults
	VehicleMovementComp->EngineSetup.MaxTorque = 1200.f;
	VehicleMovementComp->EngineSetup.EngineIdleRPM = 1500.f;
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 800.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1000.0f, 9000.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(2000.0f, 1000.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(5500.0f, 1193.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(8000.0f, 1000.0f);
	
	
	//Steering value defaults
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->Reset();
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(40.0f, 0.7f);
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(120.0f, 0.6f);

	//Differential value defaults
	VehicleMovementComp->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
	VehicleMovementComp->DifferentialSetup.FrontRearSplit = 0.7f;

	//Gearbox value defaults
	VehicleMovementComp->TransmissionSetup.bUseAutomaticGears = true;
	VehicleMovementComp->TransmissionSetup.GearChangeTime = 0.25f;

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
	Super::BeginPlay();
}

void ABaseVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}



