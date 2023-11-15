// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVehiclePawn.h"
#include "HealthComponent.h"
#include "Components/AudioComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/SpringArmComponent.h"

ABaseVehiclePawn::ABaseVehiclePawn()
{
    //Get Vehicle Movement Component when its needed
	VehicleMovementComp = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	//Engine value defaults
	VehicleMovementComp->EngineSetup.MaxTorque = 800.f;
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

	//Creates Audio Component for Engine or something...
	EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioSource"));
	
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

	//Create Bumper Collision Component
	BumperCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bumper"));
	BumperCollisionBox->SetupAttachment(RootComponent);
	BumperCollisionBox->OnComponentHit.AddDynamic(this, &ABaseVehiclePawn::OnBumperHit);
}

void ABaseVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	if(EngineAudioComponent)
	{
		EngineAudioComponent->SetSound(EngineAudioSound);
		EngineAudioComponent->SetVolumeMultiplier(1);
		EngineAudioComponent->SetActive(bPlayEngineSound);
	}
}

void ABaseVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//Magic numbers are minimum and maximum frequency for given sound.
	const float MappedEngineRotationSpeed = FMath::GetMappedRangeValueClamped(FVector2d(VehicleMovementComp->EngineSetup.EngineIdleRPM, VehicleMovementComp->GetEngineMaxRotationSpeed()),
		FVector2d(74.0f, 375.0f),
		VehicleMovementComp->GetEngineRotationSpeed());
	EngineAudioComponent->SetFloatParameter(TEXT("Frequency"), MappedEngineRotationSpeed);
	
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("BOOST AMOUNT: %f"), Booster.BoostAmount)); //DEBUG FÖR BOOST AMOUNT
}

void ABaseVehiclePawn::OnBoostPressed()
{
	if(Booster.BoostAmount <= 0) return;
	BoostStartEvent();
	//DELAY??
	Booster.SetEnabled(true);
	OnBoosting();
}

void ABaseVehiclePawn::OnBoostReleased()
{
	Booster.SetEnabled(false);
}

void ABaseVehiclePawn::OnBoosting()
{
	if(!Booster.bEnabled || Booster.BoostAmount <= 0)
	{
		VehicleMovementComp->SetMaxEngineTorque(Booster.DefaultTorque);
		VehicleMovementComp->SetThrottleInput(0);
		RechargeBoost();
		BoostStopEvent();
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, TEXT("BOOSTING")); 
	VehicleMovementComp->SetMaxEngineTorque(BoostMaxTorque);
	VehicleMovementComp->SetThrottleInput(1);
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount-BoostCost, 0.f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.BoostTimer, this, &ABaseVehiclePawn::OnBoosting, BoostConsumptionRate, true);
}

void ABaseVehiclePawn::RechargeBoost()
{
	if(Booster.bEnabled || Booster.BoostAmount >= Booster.MaxBoostAmount) return;
	
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount+BoostRechargeAmount, 0.0f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.RechargeTimer, this, &ABaseVehiclePawn::RechargeBoost, BoostRechargeRate, true);
}

void ABaseVehiclePawn::SetBoostAmount(float NewAmount)
{
	Booster.BoostAmount=NewAmount;
}

float ABaseVehiclePawn::GetBoostPercentage() const
{
	return Booster.BoostAmount/Booster.MaxBoostAmount;
}


float ABaseVehiclePawn::GetDamage()
{
	return Damage;
}

void ABaseVehiclePawn::SetDamage(float NewDamage)
{
	Damage = NewDamage;
}

void ABaseVehiclePawn::ApplyDamageBoost(float NewDamage, float TimerDuration)
{
	float OriginalDamage = Damage;
	Damage = NewDamage;
	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, "RemoveDamageBoost", OriginalDamage);
	//set timer to clear effect
	GetWorld()->GetTimerManager().SetTimer(DamageBoostTimerHandle, Delegate,TimerDuration,false);
}

void ABaseVehiclePawn::RemoveDamageBoost(float OriginalDamage)
{
	Damage = OriginalDamage;
}

APlayerTurret* ABaseVehiclePawn::GetTurret() const
{
	return Turret;
}

AMinigun* ABaseVehiclePawn::GetMinigun() const
{
	return Minigun;
}

AHomingMissileLauncher* ABaseVehiclePawn::GetHomingLauncher() const
{
	return HomingLauncher;
}

void ABaseVehiclePawn::OnBumperHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult)
{
	GEngine->AddOnScreenDebugMessage(0, 3, FColor::Cyan, TEXT("Hit"));
	if( ABaseVehiclePawn* OtherVehicle = Cast<ABaseVehiclePawn>(OtherActor) )
	{
		const double ForwardVelocity = UE::Geometry::Dot(VehicleMovementComp->Velocity, VehicleMovementComp->Velocity.ForwardVector);
		OtherVehicle->TakeDamage(ForwardVelocity / 1000.f, FDamageEvent(), nullptr, this);
		GEngine->AddOnScreenDebugMessage(0, 3, FColor::Cyan, TEXT("Damage"));
	}
}
