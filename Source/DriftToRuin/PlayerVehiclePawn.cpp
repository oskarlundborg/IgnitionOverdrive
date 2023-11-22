// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVehiclePawn.h"
#include "EnhancedInputComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HomingMissileLauncher.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"

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

	VehicleMovementComp->UpdatedPrimitive->SetPhysicsMaxAngularVelocityInDegrees(180);
	
	if(PlayerTurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;
	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Root_Turret"));
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

		EnhancedInputComponent->BindAction(AirRollYawAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyAirRollYaw);
		EnhancedInputComponent->BindAction(AirRollYawAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyAirRollYaw);

		EnhancedInputComponent->BindAction(AirRollRollAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyAirRollRoll);
		EnhancedInputComponent->BindAction(AirRollRollAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyAirRollRoll);

		EnhancedInputComponent->BindAction(AirRollPitchAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyAirRollPitch);
		EnhancedInputComponent->BindAction(AirRollPitchAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyAirRollPitch);
		

		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireMinigun);
		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireMinigunCompleted);

		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireHomingMissiles);
		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireHomingMissilesCompleted);
	}
}


void APlayerVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*
	 *Air rolling stuff, tar sönder annan nödvändig fysik på bilen
	 *
	if(IsGrounded() || !VehicleMovementComp->IsFlying()) bCanAirRoll = false;
	
	else if(!IsGrounded() && bCanAirRoll == false)
	{
		GetWorld()->GetTimerManager().SetTimer(AirRollTimer, this, &APlayerVehiclePawn::SetCanAirRollTrue, 0.5, false);
	}

	*/
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

void APlayerVehiclePawn::ApplyAirRollYaw(const FInputActionValue& Value)
{
	if(IsGrounded())
	{
		bCanAirRoll = false;
	}
	
	if(!IsGrounded() || Value.Get<float>() == 0.f && bCanAirRoll)
	{
		//Förstör resten av bilens fysik :,,))
		//GEngine->AddOnScreenDebugMessage(-1, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), FColor::Green, TEXT("YAWING"));
		//VehicleMovementComp->UpdatedPrimitive->SetPhysicsAngularVelocityInDegrees(FVector(0, 0, Value.Get<float>() * AirRollSensitivity), true, TEXT("Root"));
	}
}

void APlayerVehiclePawn::ApplyAirRollRoll(const FInputActionValue& Value)
{
	if(IsGrounded())
	{
		bCanAirRoll = false;
	}
	
	else if(!IsGrounded() || Value.Get<float>() == 0.f && bCanAirRoll)
	{
		//Förstör resten av bilens fysik :,,))
		//GEngine->AddOnScreenDebugMessage(-1, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), FColor::Green, TEXT("ROLLING"));
		//VehicleMovementComp->UpdatedPrimitive->SetPhysicsAngularVelocityInDegrees(FVector(0, Value.Get<float>() * AirRollSensitivity, 0), true, TEXT("Root"));
	}
}

void APlayerVehiclePawn::ApplyAirRollPitch(const FInputActionValue& Value)
{
	if(IsGrounded())
	{
		bCanAirRoll = false;
	}
	
	if(!IsGrounded() || Value.Get<float>() == 0.f && bCanAirRoll)
	{
		//Förstör resten av bilens fysik :,,))
		//GEngine->AddOnScreenDebugMessage(-1, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), FColor::Green, TEXT("PITCHING"));
		//VehicleMovementComp->UpdatedPrimitive->SetPhysicsAngularVelocityInDegrees(FVector(Value.Get<float>() * AirRollSensitivity, 0, 0), true, TEXT("Root"));
	}
}


void APlayerVehiclePawn::FireMinigun()
{
	if(MinigunClass == nullptr || Minigun == nullptr) return;
	Minigun->PullTrigger();
}

void APlayerVehiclePawn::FireMinigunCompleted()
{
	if(MinigunClass == nullptr || Minigun == nullptr) return;
	Minigun->ReleaseTrigger();
}

void APlayerVehiclePawn::FireHomingMissiles()
{
	if(HomingLauncherClass == nullptr || HomingLauncher == nullptr) return;
	HomingLauncher->PullTrigger();
}

void APlayerVehiclePawn::FireHomingMissilesCompleted()
{
	if(HomingLauncherClass == nullptr || HomingLauncher == nullptr) return;
	HomingLauncher->ReleaseTrigger();
}

float APlayerVehiclePawn::GetMinigunOverheatPercent() const
{
	if(MinigunClass == nullptr || Minigun == nullptr) return 0;
	return Minigun->GetOverheatValue() / Minigun->GetOverheatMaxValue();
}

bool APlayerVehiclePawn::GetMinigunIsOverheated() const
{
	if(MinigunClass == nullptr || Minigun == nullptr) return false;
	return Minigun->GetIsOverheated();
}

float APlayerVehiclePawn::GetMissileChargePercent() const
{
	if(HomingLauncherClass == nullptr || HomingLauncher == nullptr) return 0;
	return HomingLauncher->GetChargeValue() / HomingLauncher->GetChargeCapValue();
}

bool APlayerVehiclePawn::GetHomingIsCharging() const
{
	if(HomingLauncherClass == nullptr || HomingLauncher == nullptr) return false;
	return HomingLauncher->IsCharging();
}

int32 APlayerVehiclePawn::GetHomingChargeAmount() const
{
	if(HomingLauncherClass == nullptr || HomingLauncher == nullptr) return 0;
	return HomingLauncher->GetChargeAmount();
}
