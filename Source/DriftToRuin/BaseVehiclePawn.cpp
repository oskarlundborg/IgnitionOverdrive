// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVehiclePawn.h"

#include "HealthComponent.h"
#include "PowerupComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

ABaseVehiclePawn::ABaseVehiclePawn()
{
    //Get Vehicle Movement Component when its needed
	VehicleMovementComp = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	//Engine value defaults
	VehicleMovementComp->EngineSetup.MaxTorque = 1000.f;
	VehicleMovementComp->EngineSetup.MaxRPM = 9000.0f;
	VehicleMovementComp->EngineSetup.EngineBrakeEffect = 0.05f;
	VehicleMovementComp->EngineSetup.EngineRevUpMOI = 5000.0f;
	VehicleMovementComp->EngineSetup.EngineRevDownRate = 2000.0f;
	VehicleMovementComp->EngineSetup.EngineIdleRPM = 1500.f;
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->Reset();
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.0f, 550.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.125f, 600.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.25f, 750.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(0.6875f, 800.0f);
	VehicleMovementComp->EngineSetup.TorqueCurve.GetRichCurve()->AddKey(1.0f, 750.0f);
	
	//Steering value defaults
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->Reset();
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(0.0f, 1.0f);
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(80.0f, 0.7f);
	VehicleMovementComp->SteeringSetup.SteeringCurve.GetRichCurve()->AddKey(160.0f, 0.6f);
	VehicleMovementComp->SteeringSetup.AngleRatio = 1.0f;

	//Differential value defaults
	VehicleMovementComp->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
	VehicleMovementComp->DifferentialSetup.FrontRearSplit = 0.7f;

	//Gearbox value defaults
	VehicleMovementComp->TransmissionSetup.bUseAutomaticGears = true;
	VehicleMovementComp->TransmissionSetup.GearChangeTime = 0.1f;
	VehicleMovementComp->TransmissionSetup.FinalRatio = 4.0f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[0] = 7.5f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[1] = 4.5f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[2] = 3.4f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[3] = 2.6f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[4] = 2.3f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[5] = 2.02f;
	VehicleMovementComp->TransmissionSetup.ForwardGearRatios[6] = 1.97f;
	VehicleMovementComp->TransmissionSetup.ReverseGearRatios[0] = 7.0f;
	VehicleMovementComp->TransmissionSetup.ReverseGearRatios[1] = 5.0f;
	VehicleMovementComp->TransmissionSetup.ChangeUpRPM = 8000.0f;
	VehicleMovementComp->TransmissionSetup.ChangeDownRPM = 5000.0f;
	VehicleMovementComp->TransmissionSetup.TransmissionEfficiency = 0.94f;

	//Vehicle setup
	VehicleMovementComp->Mass = 2500.0f;
	VehicleMovementComp->bEnableCenterOfMassOverride = true;
	VehicleMovementComp->CenterOfMassOverride = FVector(0, 0, -35.0f);
	VehicleMovementComp->ChassisWidth = 246.0f;
	VehicleMovementComp->ChassisHeight = 254.0f;
	VehicleMovementComp->DragCoefficient = 0.5f;
	VehicleMovementComp->DownforceCoefficient = 2.0f;

	//WINGS?
	VehicleMovementComp->TorqueControl.Enabled = true;
	VehicleMovementComp->TorqueControl.YawTorqueScaling = 400.0f;
	VehicleMovementComp->TorqueControl.YawFromRollTorqueScaling = 50.0f;

	//Creates Health Component and sets it max health value
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetMaxHealth(MaxHealth);

	PowerupComponent = CreateDefaultSubobject<UPowerupComponent>(TEXT("PowerupComponent"));
	PowerupComponent->Owner = this;

	//Creates Audio Component for Engine or something...
	EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioSource"));

	//Creates Audio component for CRAZY EXPLOSIVE BOOSTS or something...
	BoostAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BoostAudioSource"));
	
	//Creates Audio Component for CRASHING or something...
	CrashAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CrashAudioSource"));

	//Audio Component for level up sound
	ScrapLevelAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ScrapLevelAudioSource"));

	//Audio component for wheel gravel and stuff.
	WheelAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WheelAudioSource"));

	//Creates Niagara system for boost vfx
	BoostVfxNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BoostNiagaraComponent"));
	BoostVfxNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Boost_Point"));

	//Create Niagara system for dirt vfx
	DirtVfxNiagaraComponentFLWheel = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DirtNiagaraComponentFL"));
	DirtVfxNiagaraComponentFLWheel->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("FL_DirtSocket"));
	DirtVfxNiagaraComponentFRWheel = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DirtNiagaraComponentFR"));
	DirtVfxNiagaraComponentFRWheel->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("FR_DirtSocket"));
	DirtVfxNiagaraComponentBLWheel = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DirtNiagaraComponentBL"));
	DirtVfxNiagaraComponentBLWheel->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("BL_DirtSocket"));
	DirtVfxNiagaraComponentBRWheel = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DirtNiagaraComponentBR"));
	DirtVfxNiagaraComponentBRWheel->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("BR_DirtSocket"));

	//Create SideThrusters
	SideThrusterL = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SideThrusterLeft"));
	SideThrusterL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("L_SideThruster"));
	SideThrusterR = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SideThrusterRight"));
	SideThrusterR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("R_SideThruster"));

	//Create Niagara system for sideswipe vfx
	SideThrusterLNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SideThrusterLNiagaraComponent"));
	SideThrusterLNiagaraComponent->AttachToComponent(SideThrusterL, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Boost_Point"));
	SideThrusterRNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SideThrusterRNiagaraComponent"));
	SideThrusterRNiagaraComponent->AttachToComponent(SideThrusterR, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Boost_Point"));
	
	//Camera & SpringArm may not be necessary in AI, move to player subclass if decided.
	
	//Create SpringArm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 500.0f;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->bInheritPitch = true;
	SpringArmComponent->bInheritYaw = true;
	SpringArmComponent->CameraLagMaxDistance = DefaultCameraLagMaxDistance;

	//Create Camera Component
	//(Camera panning constraints will be determined using Camera Manager)
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->FieldOfView = DefaultCameraFOV;

	//Create Bumper Collision Component
	BumperCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Bumper"));
	BumperCollision->SetupAttachment(RootComponent);
	BumperCollision->SetRelativeLocation({275,0,110});
	BumperCollision->SetRelativeRotation({90,90,0});
	BumperCollision->SetRelativeScale3D({1.5,3.25,2.5});
	BumperCollision->SetNotifyRigidBodyCollision(true);
	BumperCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseVehiclePawn::OnBeginOverlap);

	HomingTargetPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Homing Targeting Point"));
	HomingTargetPoint->SetupAttachment(RootComponent);
	

	//Create shield powerup mesh (hidden and ignored unless powerup active)
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(CameraComponent);
	ShieldMesh->SetRelativeLocation({2000,0,-67});
	ShieldMesh->SetRelativeRotation({0,180,0});
	ShieldMesh->SetRelativeScale3D({10,10,5});
	Hide(ShieldMesh, true);

	ExhaustL = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ExhaustL"));
	ExhaustL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("ExhaustLSocket"));
	Hide(ExhaustL, false);

	ExhaustR = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ExhaustR"));
	ExhaustR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("ExhaustRSocket"));
	Hide(ExhaustR, false);

	SpikeL = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpikeL"));
	SpikeL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("SpikeLSocket"));
	Hide(SpikeL, true);

	SpikeR = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpikeR"));
	SpikeR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("SpikeRSocket"));
	Hide(SpikeR, true);

	FuelTank = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FuelTank"));
	FuelTank->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("FuelTankSocket"));
	Hide(FuelTank, true);

	Plow = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Plow"));
	Plow->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("PlowSocket"));
	Hide(Plow, true);

	HudCapFL = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HudCapFL"));
	HudCapFL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("FL_WheelRotatorSocket"));
	Hide(HudCapFL, true);

	HudCapBL = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HudCapBL"));
	HudCapBL->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("BL_WheelRotatorSocket"));
	Hide(HudCapBL, true);

	HudCapFR = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HudCapFR"));
	HudCapFR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("FR_WheelRotatorSocket"));
	Hide(HudCapFR, true);

	HudCapBR = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HudCapBR"));
	HudCapBR->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("BR_WheelRotatorSocket"));
	Hide(HudCapBR, true);

	Windshield = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Windshield"));
	Windshield->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("WindshieldSocket"));
	Hide(Windshield, true);

	ScrapStartNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_ScrapFirst"));
	ScrapStartNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Root"));

	ScrapExplosionNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_ScrapMid"));
	ScrapExplosionNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Root"));

	ScrapIntroNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_ScrapLast"));
	ScrapIntroNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Root"));
	


	GetMesh()->OnComponentHit.AddDynamic(this, &ABaseVehiclePawn::OnHit);
	MeshDeformer = CreateDefaultSubobject<UDeformationComponent>(TEXT("Mesh Deformer"));
	MeshDeformer->AddMesh(GetMesh());
	MeshDeformer->AddMesh(SideThrusterL);
	MeshDeformer->AddMesh(SideThrusterR);
	MeshDeformer->AddMesh(ExhaustL);
	MeshDeformer->AddMesh(ExhaustR);
	MeshDeformer->AddMesh(SpikeL);
	MeshDeformer->AddMesh(SpikeR);
	MeshDeformer->AddMesh(HudCapBR);
	MeshDeformer->AddMesh(HudCapFR);
	MeshDeformer->AddMesh(HudCapBL);
	MeshDeformer->AddMesh(HudCapFL);
	MeshDeformer->AddMesh(FuelTank);
	MeshDeformer->AddMesh(Plow);
	MeshDeformer->AddMesh(Windshield);
	MeshDeformer->BoneIgnoreFilter = {
		TEXT("FL_WheelRotator"),
		TEXT("FL_SwayBar"),
		TEXT("FL_End"),
		TEXT("FR_WheelRotator"),
		TEXT("FR_SwayBar"),
		TEXT("FR_End"),
		TEXT("BL_WheelRotator"),
		TEXT("BL_SwayBar"),
		TEXT("BL_End"),
		TEXT("BR_WheelRotator"),
		TEXT("BR_SwayBar"),
		TEXT("BR_End"),
		TEXT("Root_Thruster"),
		TEXT("BL_ThrusterHatchet"),
		TEXT("BR_ThrusterHatchet"),
		TEXT("TL_ThrusterHatchet"),
		TEXT("TR_ThrusterHatchet"),
	};
}

void ABaseVehiclePawn::BeginPlay()
{
	Super::BeginPlay();
	
	InitAudio();
	InitVFX();
}

FVector ABaseVehiclePawn::GetCameraLocation() const
{
	return CameraComponent->GetComponentLocation();
}

UCameraComponent* ABaseVehiclePawn::GetCameraComponent() const
{
	return CameraComponent;
}

void ABaseVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateEngineSFX();
	UpdateWheelSFX();
	UpdateGravelVFX();
	UpdateAirbornePhysics();
	
	//GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("IS GROUNDED: %d"), IsGrounded()));
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("BOOST AMOUNT: %f"), Booster.BoostAmount)); //DEBUG FÖR BOOST AMOUNT
}

void ABaseVehiclePawn::OnBoostPressed()
{
	if(Booster.BoostAmount <= 0) return;
	BoostAudioComponent->Play();
	bCanFadeOutBoost = true;
	bBoostReleased = false;
	GetWorld()->GetTimerManager().SetTimer(BoostCooldownTimer, this, &ABaseVehiclePawn::EnableBoost, 0.7f, false);
	
	
}

void ABaseVehiclePawn::OnBoostReleased()
{
	bBoostReleased = true;
	DisableBoost();
}

void ABaseVehiclePawn::OnBoosting()
{
	if(Booster.bEnabled && Booster.BoostAmount <= 0)
	{
		DisableBoost();
		return;
	}
	if(!Booster.bEnabled)
	{
		if(bUseCrazyCamera)
		{
			SpringArmComponent->CameraLagMaxDistance = FMath::FInterpTo(SpringArmComponent->CameraLagMaxDistance, DefaultCameraLagMaxDistance, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), BoostEndCameraInterpSpeed);
			CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, DefaultCameraFOV, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), BoostEndCameraInterpSpeed));
		}
		return;
	}
	if(bUseCrazyCamera)
	{
		SpringArmComponent->CameraLagMaxDistance = FMath::FInterpTo(SpringArmComponent->CameraLagMaxDistance, BoostCameraLagMaxDistance, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), BoostCameraInterpSpeed);
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, BoostCameraFOV, UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), BoostCameraInterpSpeed));
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, TEXT("BOOSTING")); 
	VehicleMovementComp->SetThrottleInput(1);
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount - BoostCost * GetWorld()->DeltaTimeSeconds, 0.f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.BoostTimer, this, &ABaseVehiclePawn::OnBoosting, BoostConsumptionRate * GetWorld()->DeltaTimeSeconds, true);
}

void ABaseVehiclePawn::EnableBoost()
{
	if(!bBoostReleased)
	{
		Booster.SetEnabled(true);
		BoostVfxNiagaraComponent->Activate(true);
		VehicleMovementComp->SetMaxEngineTorque(BoostMaxTorque);
		VehicleMovementComp->SetDownforceCoefficient(4);
		for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
		{
			if(Wheel->AxleType==EAxleType::Rear)
			{
				VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, BoostRearFrictionForce);
				VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 0.85);
			}
			else
			{
				VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, BoostFrontFrictionForce);
				VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 0.92);
			}
		}
		if(bUseCrazyCamera)
		{
			APlayerController* PController = Cast<APlayerController>(GetController());
			if(BoostCameraShake != nullptr) PController->PlayerCameraManager->StartCameraShake(BoostCameraShake, 1);
		}
		OnBoosting();
	}
	
}

void ABaseVehiclePawn::DisableBoost()
{
	Booster.SetEnabled(false);
	VehicleMovementComp->SetMaxEngineTorque(Booster.DefaultTorque);
	VehicleMovementComp->SetDownforceCoefficient(2);
	VehicleMovementComp->SetThrottleInput(0);
	BoostVfxNiagaraComponent->Deactivate();
	if(bCanFadeOutBoost)
	{
		BoostAudioComponent->SetTriggerParameter(TEXT("StopBoost"));
	}
	bCanFadeOutBoost = false;
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		if(Wheel->AxleType==EAxleType::Rear)
		{
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, 6.0f);
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 1);
		}
		else
		{
			VehicleMovementComp->SetWheelFrictionMultiplier(Wheel->WheelIndex, 5.6f);
			VehicleMovementComp->SetWheelSlipGraphMultiplier(Wheel->WheelIndex, 1);
		}
	}
	RechargeBoost();
}

void ABaseVehiclePawn::SetBoostCost(float NewBoostCost)
{
	BoostCost = NewBoostCost;
}

void ABaseVehiclePawn::ResetBoostCost()
{
	BoostCost = DefaultBoostCost;
}

void ABaseVehiclePawn::RechargeBoost()
{
	if(Booster.bEnabled || Booster.BoostAmount >= Booster.MaxBoostAmount) return;
	
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount+BoostRechargeAmount * GetWorld()->DeltaTimeSeconds, 0.0f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.RechargeTimer, this, &ABaseVehiclePawn::RechargeBoost, BoostRechargeRate * GetWorld()->DeltaTimeSeconds, true);
}

bool ABaseVehiclePawn::IsGrounded() const
{
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		if(!Wheel->IsInAir())
		{
			return true;
		}
	}
	return false;
}

void ABaseVehiclePawn::UpdateGravelVFX() const
{
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		switch(Wheel->WheelIndex)
		{
		case 0: 
			DirtVfxNiagaraComponentFLWheel->SetActive(!Wheel->IsInAir() && Wheel->GetContactSurfaceMaterial()->SurfaceType == SurfaceType2);
			break;
		case 1: 
			DirtVfxNiagaraComponentFRWheel->SetActive(!Wheel->IsInAir() && Wheel->GetContactSurfaceMaterial()->SurfaceType == SurfaceType2);
			break;
		case 2: 
			DirtVfxNiagaraComponentBLWheel->SetActive(!Wheel->IsInAir() && Wheel->GetContactSurfaceMaterial()->SurfaceType == SurfaceType2);
			break;
		case 3: 
			DirtVfxNiagaraComponentBRWheel->SetActive(!Wheel->IsInAir() && Wheel->GetContactSurfaceMaterial()->SurfaceType == SurfaceType2);
			break;
		default:
			break;
		}
	}
}

void ABaseVehiclePawn::UpdateAirbornePhysics() const
{
	if(!IsGrounded())
	{
		if(!Booster.bEnabled)
		{
			GetMesh()->SetLinearDamping(0.2f);
			GetMesh()->SetAngularDamping(0.3f);
			VehicleMovementComp->SetDownforceCoefficient(AirborneDownforceCoefficient);
			//GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("AIRBORNE NOT BOOSTING")));
		}
		else
		{
			GetMesh()->SetLinearDamping(0.05f);
			GetMesh()->SetAngularDamping(0.3f);
			//GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("AIRBORNE BOOSTING")));
		}
		
	}
	else if(IsGrounded())
	{
		GetMesh()->SetLinearDamping(0.01f);
		GetMesh()->SetAngularDamping(0.0f);
		VehicleMovementComp->SetDownforceCoefficient(VehicleMovementComp->DownforceCoefficient);
	}
}

void ABaseVehiclePawn::UpdateEngineSFX() const
{
	//Magic numbers are minimum and maximum frequency for given sound.
	const float MappedEngineRotationSpeed = FMath::GetMappedRangeValueClamped(FVector2d(VehicleMovementComp->EngineSetup.EngineIdleRPM, VehicleMovementComp->GetEngineMaxRotationSpeed()),
		FVector2d(0.0f, 1.0f),
		VehicleMovementComp->GetEngineRotationSpeed());
	EngineAudioComponent->SetFloatParameter(TEXT("EngineRPM"), MappedEngineRotationSpeed);
}

void ABaseVehiclePawn::UpdateWheelSFX() const
{

	WheelAudioComponent->SetPaused(!IsGrounded());
	
	const float MappedForwardSpeed = FMath::GetMappedRangeValueClamped(FVector2d(0, 100.0f),
		FVector2d(0.0f, 2.0f),
		VehicleMovementComp->GetForwardSpeedMPH());
	WheelAudioComponent->SetFloatParameter(TEXT("ForwardSpeed"),  MappedForwardSpeed);
	
	for(UChaosVehicleWheel* Wheel : VehicleMovementComp->Wheels)
	{
		if(VehicleMovementComp->GetWheelState(Wheel->WheelIndex).bIsSkidding || VehicleMovementComp->GetWheelState(Wheel->WheelIndex).bIsSlipping)
		{
			WheelAudioComponent->SetFloatParameter(TEXT("SlideModifier"), 2);
			return;
		}
	}
	WheelAudioComponent->SetFloatParameter(TEXT("SlideModifier"), 1);
}	


void ABaseVehiclePawn::InitVFX()
{
	if(BoostVfxNiagaraComponent)
	{
		BoostVfxNiagaraComponent->SetAsset(BoostVfxNiagaraSystem);
		BoostVfxNiagaraComponent->Deactivate();
	}

	if(DirtVfxNiagaraComponentFLWheel)
	{
		DirtVfxNiagaraComponentFLWheel->SetAsset(DirtVfxNiagaraSystem);
		DirtVfxNiagaraComponentFLWheel->Deactivate();
	}
	if(DirtVfxNiagaraComponentFRWheel)
	{
		DirtVfxNiagaraComponentFRWheel->SetAsset(DirtVfxNiagaraSystem);
		DirtVfxNiagaraComponentFRWheel->Deactivate();
	}
	if(DirtVfxNiagaraComponentBLWheel)
	{
		DirtVfxNiagaraComponentBLWheel->SetAsset(DirtVfxNiagaraSystem);
		DirtVfxNiagaraComponentBLWheel->Deactivate();
	}
	if(DirtVfxNiagaraComponentBRWheel)
	{
		DirtVfxNiagaraComponentBRWheel->SetAsset(DirtVfxNiagaraSystem);
		DirtVfxNiagaraComponentBRWheel->Deactivate();
	}
	if(SideThrusterLNiagaraComponent)
	{
		SideThrusterLNiagaraComponent->SetAsset(SideSwipeVfxNiagaraSystem);
		SideThrusterLNiagaraComponent->Deactivate();
	}
	if(SideThrusterRNiagaraComponent)
	{
		SideThrusterRNiagaraComponent->SetAsset(SideSwipeVfxNiagaraSystem);
		SideThrusterRNiagaraComponent->Deactivate();
	}
}

void ABaseVehiclePawn::InitAudio()
{
	if(EngineAudioComponent)
	{
		EngineAudioComponent->SetSound(EngineAudioSound);
		EngineAudioComponent->SetVolumeMultiplier(1);
		EngineAudioComponent->SetActive(bPlayEngineSound);
	}
	if(CrashAudioComponent)
	{
		CrashAudioComponent->SetSound(CrashAudioSound);
		CrashAudioComponent->SetVolumeMultiplier(1);
		CrashAudioComponent->SetActive(bPlayCrashSound);
		CrashAudioComponent->Stop();
	}
	if(BoostAudioComponent)
	{
		BoostAudioComponent->SetSound(BoostAudioSound);
		BoostAudioComponent->SetVolumeMultiplier(1);
		BoostAudioComponent->SetActive(bPlayEngineSound);
		BoostAudioComponent->Stop();
	}
	if(WheelAudioComponent)
	{
		WheelAudioComponent->SetSound(WheelAudioSound);
		WheelAudioComponent->SetVolumeMultiplier(1);
		WheelAudioComponent->SetActive(bPlayEngineSound);
	}
}


void ABaseVehiclePawn::SetBoostAmount(float NewAmount)
{
	Booster.BoostAmount=NewAmount;
}

float ABaseVehiclePawn::GetMaxBoostAmount()
{
	return Booster.MaxBoostAmount;
}

float ABaseVehiclePawn::GetBoostPercentage() const
{
	return Booster.BoostAmount/Booster.MaxBoostAmount;
}

bool ABaseVehiclePawn::GetIsBoostEnabled() const
{
	return Booster.bEnabled;
}

float ABaseVehiclePawn::GetMinigunDamage()
{
	return MinigunDamage;
}

float ABaseVehiclePawn::GetMinigunDefaultDamage()
{
    return DefaultMinigunDamage;
}

float ABaseVehiclePawn::GetHomingDamage()
{
	return HomingDamage;
}

void ABaseVehiclePawn::SetDamage(float NewDamage)
{
	MinigunDamage = NewDamage;
}

void ABaseVehiclePawn::SetMinigunDamage(int NewDamage)
{
	MinigunDamage = NewDamage;
}

void ABaseVehiclePawn::ApplyDamageBoost(float NewDamage, float TimerDuration)
{
	float OriginalDamage = MinigunDamage;
	MinigunDamage = NewDamage;
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "RemoveDamageBoost", OriginalDamage);
	//set timer to clear effect
	GetWorld()->GetTimerManager().SetTimer(DamageBoostTimerHandle, Delegate,TimerDuration,false);
}

void ABaseVehiclePawn::RemoveDamageBoost(float OriginalDamage)
{
	MinigunDamage = OriginalDamage;
}

UHealthComponent *ABaseVehiclePawn::GetHealthComponent()
{
    return HealthComponent;
}

UStaticMeshComponent *ABaseVehiclePawn::GetShieldMeshComponent()
{
    return ShieldMesh;
}

bool ABaseVehiclePawn::GetIsDead()
{
	return HealthComponent->IsDead();
}

/*APlayerTurret* ABaseVehiclePawn::GetTurret() const
{
	return Turret;
}*/

AMinigun* ABaseVehiclePawn::GetMinigun() const
{
	return Minigun;
}

AHomingMissileLauncher* ABaseVehiclePawn::GetHomingLauncher() const
{
	return HomingLauncher;
}

float ABaseVehiclePawn::GetScrapPercentage()
{
    return ScrapAmount / MaxScrap;
}

void ABaseVehiclePawn::AddScrapAmount(float Scrap, float HealAmount)
{
	ScrapAmount += Scrap;
	HealthComponent->SetHealth(HealthComponent->GetHealth() + HealAmount);
	CheckScrapLevel();
}

void ABaseVehiclePawn::RemoveScrapAmount(float Scrap)
{
	ScrapAmount = FMath::Clamp(ScrapAmount - Scrap, 0.f, 100.f);
}

float ABaseVehiclePawn::GetScrapToDrop()
{
    return ScrapToDrop;
}

int ABaseVehiclePawn::GetKillpointWorth()
{
    return KillpointWorth;
}

void ABaseVehiclePawn::ResetScrapLevel()
{
	MinigunDamage = DefaultMinigunDamage;
	HomingDamage = DefaultHomingDamage;
	HealthComponent->ResetMaxHealth();
	HealthComponent->SetHealth(HealthComponent->GetDefaultMaxHealth());
	KillpointWorth = 1;
	ScrapToDrop = 10;
	bHitLevelOne = false;
	bHitLevelTwo = false;
	bHitLevelThree = false;
	ScrapAmount = 0;
	HitScrapLevelThree(false);
	

	Hide(SpikeL, true);
	Hide(SpikeR, true);
	Hide(HudCapBL,true);
	Hide(HudCapBR, true);
	Hide(HudCapFR, true);
	Hide(HudCapFL, true);

	Hide(ExhaustL, true);
	Hide(ExhaustR, true);
	Hide(FuelTank, true);

	Hide(Plow, true);
}

UPowerupComponent *ABaseVehiclePawn::GetPowerupComponent()
{
    return PowerupComponent;
}

void ABaseVehiclePawn::ActivatePowerup()
{
	
	//Pickup.Vehicle = this;

	switch (HeldPowerup)
	{
	case 1:
		PowerupComponent->HealthPowerup(); //Pickup.HealthPowerup(); //Regenerate Health
		SetHeldPowerup(0);
		break;
	case 2:
		PowerupComponent->BoostPowerup(); //Pickup.BoostPowerup(); //Infinite boost
		SetHeldPowerup(0);
		break;
	case 3:
		PowerupComponent->OverheatPowerup(); //Pickup.OverheatPowerup(); //No overheat
		SetHeldPowerup(0);
		break;
	case 4:
		PowerupComponent->ShieldPowerup(); //Pickup.ShieldPowerup(); //skapar shield
		SetHeldPowerup(0);
		break;
	default:
		break;
	}
}

void ABaseVehiclePawn::SetHeldPowerup(int PowerIndex)
{
	HeldPowerup = PowerIndex;
}

void ABaseVehiclePawn::CheckScrapLevel()
{
	if (ScrapAmount < 20)
	{
		KillpointWorth = 1;
		ScrapToDrop = 10;
	}
	
	if (ScrapAmount >= 20 && ScrapAmount < 50 && !bHitLevelOne)
	{
		KillpointWorth = 2;
		ScrapToDrop = 15;
		MinigunDamage = MinigunDamage * 1.1;
		HomingDamage = HomingDamage * 1.1;
		HealthComponent->SetMaxHealth(HealthComponent->GetDefaultMaxHealth() *  1.1);
		HealthComponent->SetHealth(HealthComponent->GetHealth() + 10);
		bHitLevelOne = true;
		ScrapLevelAudioComponent->Play();

		Hide(SpikeL, false);
		Hide(SpikeR, false);
		Hide(HudCapBL,false);
		Hide(HudCapBR, false);
		Hide(HudCapFR, false);
		Hide(HudCapFL, false);
		//wheel spikes, spikes
	}

	if (ScrapAmount >= 50 && ScrapAmount < 100 && !bHitLevelTwo)
	{	
		KillpointWorth = 3;
		ScrapToDrop = 25;
		MinigunDamage = MinigunDamage * 1.25;
		HomingDamage = HomingDamage * 1.25;
		HealthComponent->SetMaxHealth(HealthComponent->GetDefaultMaxHealth() *  1.25);
		HealthComponent->SetHealth(HealthComponent->GetHealth() + 15);
		bHitLevelTwo = true;
		ScrapLevelAudioComponent->Play();

		Hide(FuelTank, false);
		//fuel tank, exhausts
	}

	if (ScrapAmount >= 100 && !bHitLevelThree)
	{
		//Aktivera en marker som visar vart spelaren är (light pillar)

		KillpointWorth = 5;
		ScrapToDrop = 40;
		MinigunDamage = MinigunDamage * 1.5;
		HomingDamage = HomingDamage * 1.5;
		HealthComponent->SetMaxHealth(HealthComponent->GetDefaultMaxHealth() *  1.5);
		HealthComponent->SetHealth(HealthComponent->GetHealth() + 25);
		bHitLevelThree = true;
		ScrapLevelAudioComponent->Play();
		HitScrapLevelThree(true);

		Hide(Plow, false);
		Hide(Windshield, false);
		// plow, wind shield
	}
}

USceneComponent* ABaseVehiclePawn::GetHomingTargetPoint() const
{
	return HomingTargetPoint;
}

void ABaseVehiclePawn::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || OverlappedComp != BumperCollision /* || (LastHitLocation - SweepResult.Location).Length() <= HitDistanceMinimum */) { return; }
	ABaseVehiclePawn* OtherVehicle = Cast<ABaseVehiclePawn>(OtherActor);
	if(OtherVehicle && !OtherVehicle->GetIsDead())
	{
		const auto Speed = GetVehicleMovement()->GetForwardSpeed();
		if( Speed < 100.f ) { return; }
		UGameplayStatics::ApplyDamage(
			OtherActor,
			FMath::Clamp(FMath::Clamp(Speed, 1.f, Speed) / BumperDamageDividedBy, 0.f, MaxBumperDamage),
			GetController(),
			this,
			nullptr
		);
	}
}

void ABaseVehiclePawn::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((LastHitLocation - Hit.Location).Length() <= HitDistanceMinimum) { return; }
	LastHitLocation = Hit.Location;
	if (!bAllowHit) { return; }
	bAllowHit = false;

	const auto Speed = VehicleMovementComp->GetForwardSpeedMPH() < 0 ? -VehicleMovementComp->GetForwardSpeedMPH() : VehicleMovementComp->GetForwardSpeedMPH();
	const auto ImpactForce = FMath::GetMappedRangeValueClamped(
		FVector2d(0, 200),
		FVector2d(0.0f, 1.0f),
		Speed
	);
	//GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Cyan, FString::Printf(TEXT("Impact force: %f"), ImpactForce));
	
	if(const APlayerController* PController = Cast<APlayerController>(GetController())) 
	{
		if(CrashCameraShake)
		{
			PController->PlayerCameraManager->StartCameraShake(
				CrashCameraShake,
				ImpactForce
			);
		}
	}
	
	if (!CrashAudioComponent->IsPlaying())
	{
		CrashAudioComponent->SetFloatParameter(TEXT("ImpactForce"), ImpactForce);
		CrashAudioComponent->Play();
		CrashAudioComponent->FadeOut(1.f, .2f, EAudioFaderCurve::Sin);
	}
	FTimerHandle THandle;
	FTimerDelegate TDelegate;
	TDelegate.BindLambda([&]
	{
		bAllowHit = true;
	});
	GetWorldTimerManager().SetTimer(THandle, TDelegate, 1.f, false);
}

void ABaseVehiclePawn::Hide(UPrimitiveComponent *Component, bool bHide)
{
	if (bHide)
	{
		Component->SetVisibility(false);
		Component->SetGenerateOverlapEvents(false);
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		Component->SetVisibility(true);
		Component->SetGenerateOverlapEvents(true);
		//Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
