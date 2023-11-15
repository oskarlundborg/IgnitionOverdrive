// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"

APlayerVehiclePawn::APlayerVehiclePawn()
{
	
}

void APlayerVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(VehicleMappingContext, 0);
		}
	}

	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Root_Turret"));
	Minigun->SetOwner(this);

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("Root_MissileLauncher"));
	HomingLauncher->SetOwner(this);
}

void APlayerVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Vehicle control axis
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Vehicle control axis
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyThrottle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyThrottle);
		EnhancedInputComponent->BindAction(BrakingAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyBraking);
		EnhancedInputComponent->BindAction(BrakingAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyBraking);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplySteering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplySteering);
		
		//Camera control axis
		EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::LookUp);
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::LookAround);

		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::OnHandbrakePressed);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::OnHandbrakeReleased);

		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::OnBoostPressed);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::OnBoostReleased);

		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireMinigun);
		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireMinigunCompleted);

		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireHomingMissiles);
		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireHomingMissilesCompleted);
	}
}


void APlayerVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void APlayerVehiclePawn::ApplyThrottle(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetThrottleInput(Value.Get<float>());
}

void APlayerVehiclePawn::ApplyBraking(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetBrakeInput(Value.Get<float>());
}

void APlayerVehiclePawn::ApplySteering(const FInputActionValue& Value)
{
	GetVehicleMovementComponent()->SetSteeringInput(Value.Get<float>());
}

void APlayerVehiclePawn::LookAround(const FInputActionValue& Value)
{
	// möjligtvis fixa olika sensitivity för upp/ner och vänster/höger för mus och kontroller

	if(Value.Get<float>() != 0.f)
	{
		AddControllerYawInput(Value.Get<float>()*Sensitivity);
	}
}

void APlayerVehiclePawn::LookUp(const FInputActionValue& Value)
{
	if(Value.Get<float>() != 0.f)
	{
		AddControllerPitchInput(Value.Get<float>()*Sensitivity);
	}
}

void APlayerVehiclePawn::OnHandbrakePressed()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(true);	
}

void APlayerVehiclePawn::OnHandbrakeReleased()
{
	GetVehicleMovementComponent()->SetHandbrakeInput(false);
}

void APlayerVehiclePawn::FireMinigun()
{
	Minigun->PullTrigger();
}

void APlayerVehiclePawn::FireMinigunCompleted()
{
	Minigun->ReleaseTrigger();
}

void APlayerVehiclePawn::FireHomingMissiles()
{
	HomingLauncher->PullTrigger();
}

void APlayerVehiclePawn::FireHomingMissilesCompleted()
{
	HomingLauncher->ReleaseTrigger();
}

float APlayerVehiclePawn::GetMinigunOverheatPercent() const
{
	return Minigun->GetOverheatValue() / Minigun->GetOverheatMaxValue();
}
