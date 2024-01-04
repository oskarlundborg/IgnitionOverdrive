// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerVehiclePawn.h"

#include "EnhancedInputComponent.h"
#include "ChaosVehicleMovementComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HomingMissileLauncher.h"
#include "TimerManager.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "Camera/CameraComponent.h"
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

	DefaultFrontFriction=VehicleMovementComp->Wheels[0]->FrictionForceMultiplier;

	DefaultRearFriction=VehicleMovementComp->Wheels[2]->FrictionForceMultiplier;
	
	if(PlayerTurretClass == nullptr || MinigunClass == nullptr || HomingLauncherClass == nullptr) return;
	Turret = GetWorld()->SpawnActor<APlayerTurret>(PlayerTurretClass);
	Turret->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("TurretRefrencJoint"));
	Turret->SetOwner(this);

	Minigun = GetWorld()->SpawnActor<AMinigun>(MinigunClass);
	Minigun->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("MinigunRef"));
	Minigun->SetOwner(this);
	Minigun->InitializeOwnerVariables();

	HomingLauncher = GetWorld()->SpawnActor<AHomingMissileLauncher>(HomingLauncherClass);
	HomingLauncher->AttachToComponent(Turret->GetTurretMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TEXT("MissileLauncerRef"));
	HomingLauncher->SetOwner(this);
	HomingLauncher->InitializeOwnerVariables();
	InitializeWeaponReferancesInBP(Minigun, HomingLauncher);
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

		EnhancedInputComponent->BindAction(SideSwipeLeftAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::SideSwipeLeft);
		EnhancedInputComponent->BindAction(SideSwipeRightAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::SideSwipeRight);
		
		//Camera control axis
		EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::LookUp);
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::LookAround);
		EnhancedInputComponent->BindAction(LookUpAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::LookUp);
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::LookAround);

		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::OnHandbrakePressed);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::OnHandbrakeReleased);

		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::OnBoostPressed);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::OnBoostReleased);

		EnhancedInputComponent->BindAction(AirRollYawAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyAirRollYaw);
		EnhancedInputComponent->BindAction(AirRollYawAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyAirRollYaw);

		EnhancedInputComponent->BindAction(AirRollPitchAction, ETriggerEvent::Triggered, this, &APlayerVehiclePawn::ApplyAirRollPitch);
		EnhancedInputComponent->BindAction(AirRollPitchAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::ApplyAirRollPitch);
		

		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireMinigun);
		EnhancedInputComponent->BindAction(FireMinigunAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireMinigunCompleted);

		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Started, this, &APlayerVehiclePawn::FireHomingMissiles);
		EnhancedInputComponent->BindAction(FireHomingMissilesAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::FireHomingMissilesCompleted);

		EnhancedInputComponent->BindAction(CameraModeToggleAction, ETriggerEvent::Completed, this, &APlayerVehiclePawn::CameraModeToggle);
	}
}


void APlayerVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	bCanAirRoll = !IsGrounded();
	GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("Camera Lock: %d"), CameraLock));
	GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("Is Moving Camera X: %d"), IsMovingCameraX));
	GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("Is Moving Camera Y: %d"), IsMovingCameraY));

	if(CameraLock && !IsMovingCameraX && !IsMovingCameraY) GetController()->SetControlRotation(FMath::RInterpTo(GetControlRotation(), GetActorForwardVector().Rotation(), DeltaSeconds, CameraLockSpeed));
	
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
	
	AddControllerYawInput(Value.Get<float>()*Sensitivity);
	GEngine->AddOnScreenDebugMessage(-1, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), FColor::Green, FString::Printf(TEXT("CameraX: %f"), Value.Get<float>()));
	IsMovingCameraX = Value.Get<float>() > 0.5f || Value.Get<float>() < -0.5f;
}

void APlayerVehiclePawn::LookUp(const FInputActionValue& Value)
{
	AddControllerPitchInput(Value.Get<float>()*Sensitivity);
	GEngine->AddOnScreenDebugMessage(-1, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), FColor::Green, FString::Printf(TEXT("CameraY: %f"), Value.Get<float>()));
	IsMovingCameraY = Value.Get<float>() > 0.5f || Value.Get<float>() < -0.5f;
	
}

void APlayerVehiclePawn::OnHandbrakePressed()
{
	VehicleMovementComp->SetHandbrakeInput(true);
	VehicleMovementComp->SetDownforceCoefficient(0.3f);
	VehicleMovementComp->SetDifferentialFrontRearSplit(1);
	
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		if(Wheel->AxleType==EAxleType::Rear)
		{
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 0.33);
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, DriftRearFriction);
		}
		else
		{
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 0.66);
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, DriftFrontFriction);
		}
	}
}

void APlayerVehiclePawn::OnHandbrakeReleased()
{
	VehicleMovementComp->SetHandbrakeInput(false);
	VehicleMovementComp->SetDownforceCoefficient(4);
	VehicleMovementComp->SetDifferentialFrontRearSplit(0.7f);
	
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		if(Wheel->AxleType==EAxleType::Rear)
		{
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 1);
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, 10.0f);
		}
		else
		{
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 1);
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, 9.6f);
		}
	}
}

void APlayerVehiclePawn::SideSwipeLeft()
{
	if(bCanSideSwipe)
	{
		bCanSideSwipe = false;
		SideThrusterRNiagaraComponent->Activate(true);
		UGameplayStatics::PlaySoundAtLocation(this, SideswipeSound, GetActorLocation());
		GetMesh()->AddImpulse(-GetActorRightVector()*SideSwipeForce, TEXT("Root"), true);
		GetWorld()->GetTimerManager().SetTimer(SideSwipeTimer, this, &APlayerVehiclePawn::SetCanSideSwipeTrue, SideSwipeCooldown, false);
	}
	
}

void APlayerVehiclePawn::SideSwipeRight()
{
	if(bCanSideSwipe)
	{
		bCanSideSwipe = false;
		SideThrusterLNiagaraComponent->Activate(true);
		UGameplayStatics::PlaySoundAtLocation(this, SideswipeSound, GetActorLocation());
		GetMesh()->AddImpulse(GetActorRightVector()*SideSwipeForce, TEXT("Root"), true);
		GetWorld()->GetTimerManager().SetTimer(SideSwipeTimer, this, &APlayerVehiclePawn::SetCanSideSwipeTrue, SideSwipeCooldown, false);
	}
}

void APlayerVehiclePawn::ApplyAirRollYaw(const FInputActionValue& Value)
{
	if(bCanAirRoll && !VehicleMovementComp->GetHandbrakeInput())
	{
		GetMesh()->AddAngularImpulseInDegrees(GetMesh()->GetUpVector() * Value.Get<float>() * AirRollSensitivity, NAME_None, true);
	}
	else if(bCanAirRoll && VehicleMovementComp->GetHandbrakeInput())
	{
		GetMesh()->AddAngularImpulseInDegrees(GetMesh()->GetForwardVector() * -Value.Get<float>() * AirRollSensitivity, NAME_None, true);
	}
}

void APlayerVehiclePawn::ApplyAirRollPitch(const FInputActionValue& Value)
{
	if(bCanAirRoll)
	{
		GetMesh()->AddAngularImpulseInDegrees(-(GetMesh()->GetRightVector() * Value.Get<float>()) * AirRollSensitivity, NAME_None, true);
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

APlayerTurret* APlayerVehiclePawn::GetTurret() const
{
	return Turret;
}

bool APlayerVehiclePawn::GetCameraLockMode() const
{
	return CameraLock;
}

void APlayerVehiclePawn::CameraModeToggle()
{
	CameraLock ^= true;
}

