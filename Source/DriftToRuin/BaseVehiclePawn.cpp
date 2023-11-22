// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVehiclePawn.h"
#include "HealthComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

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

	//Creates Niagara system for boost vfx
	BoostVfxNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BoostNiagaraComponent"));
	BoostVfxNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("Boost_Point"));
	
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
	BumperCollisionBox->SetRelativeLocation({285,0,0});
	BumperCollisionBox->SetRelativeScale3D({1,3.25,0.75});
	BumperCollisionBox->SetNotifyRigidBodyCollision(true);
	BumperCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABaseVehiclePawn::OnBumperBeginOverlap);
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

	if(BoostVfxNiagaraComponent)
	{
		BoostVfxNiagaraComponent->SetAsset(BoostVfxNiagaraSystem);
		BoostVfxNiagaraComponent->Deactivate();
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

	if(!IsGrounded())
	{
		if(!Booster.bEnabled)
		{
			VehicleMovementComp->SetDownforceCoefficient(AirborneDownforceCoefficient);
			GetMesh()->SetLinearDamping(0.2f);
			GetMesh()->SetAngularDamping(0.3f);
		}
		else
		{
			GetMesh()->SetLinearDamping(0.05f);
			GetMesh()->SetAngularDamping(0.3f);
		}
		
	}
	else if(IsGrounded())
	{
		GetMesh()->SetLinearDamping(0.01f);
		GetMesh()->SetAngularDamping(0.0f);
		VehicleMovementComp->SetDownforceCoefficient(VehicleMovementComp->DownforceCoefficient);
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("IS GROUNDED: %d"), IsGrounded()));
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("BOOST AMOUNT: %f"), Booster.BoostAmount)); //DEBUG FÖR BOOST AMOUNT
}

void ABaseVehiclePawn::OnBoostPressed()
{
	if(Booster.BoostAmount <= 0) return;
	BoostStartEvent();
	BoostVfxNiagaraComponent->Activate(true);
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
		BoostVfxNiagaraComponent->Deactivate();
		BoostStopEvent();
		return;
	}
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Cyan, TEXT("BOOSTING")); 
	VehicleMovementComp->SetMaxEngineTorque(BoostMaxTorque);
	VehicleMovementComp->SetThrottleInput(1);
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount-BoostCost, 0.f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.BoostTimer, this, &ABaseVehiclePawn::OnBoosting, BoostConsumptionRate*UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), true);
}

void ABaseVehiclePawn::RechargeBoost()
{
	if(Booster.bEnabled || Booster.BoostAmount >= Booster.MaxBoostAmount) return;
	
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount+BoostRechargeAmount, 0.0f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.RechargeTimer, this, &ABaseVehiclePawn::RechargeBoost, BoostRechargeRate*UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), true);
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


void ABaseVehiclePawn::SetBoostAmount(float NewAmount)
{
	Booster.BoostAmount=NewAmount;
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

float ABaseVehiclePawn::GetHomingDamage()
{
	return HomingDamage;
}

void ABaseVehiclePawn::SetDamage(float NewDamage)
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

bool ABaseVehiclePawn::GetIsDead()
{
	return HealthComponent->IsDead();
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
	ScrapAmount = FMath::Clamp(ScrapAmount - Scrap, 0, 100);
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
	}

	if (ScrapAmount == 100 && !bHitLevelThree)
	{
		//Aktivera en marker som visar vart spelaren är (light pillar)

		KillpointWorth = 5;
		ScrapToDrop = 40;
		MinigunDamage = MinigunDamage * 1.5;
		HomingDamage = HomingDamage * 1.5;
		HealthComponent->SetMaxHealth(HealthComponent->GetDefaultMaxHealth() *  1.5);
		HealthComponent->SetHealth(HealthComponent->GetHealth() + 25);
		bHitLevelThree = true;
	}
	
	
	
}

void ABaseVehiclePawn::OnBumperBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if( OtherActor == this ) { return; }
	if( ABaseVehiclePawn* OtherVehicle = Cast<ABaseVehiclePawn>(OtherActor) )
	{
		if( bFlatDamage && VehicleMovementComp->GetForwardSpeed() > 0.01f )
		{
			OtherVehicle->TakeDamage(BumperDamage, FDamageEvent(), nullptr, this);
			//GEngine->AddOnScreenDebugMessage(0, 3, FColor::Cyan, FString::Printf(TEXT("Damage: %f"), BumperDamage));
		}
		else
		{
			OtherVehicle->TakeDamage(VehicleMovementComp->GetForwardSpeed() * DamageMultiplier, FDamageEvent(), nullptr, this);
			//GEngine->AddOnScreenDebugMessage(0, 3, FColor::Cyan, FString::Printf(TEXT("Damage: %f"), VehicleMovementComp->GetForwardSpeed() * DamageMultiplier));
		}
	}
}
