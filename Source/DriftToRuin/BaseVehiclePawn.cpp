// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseVehiclePawn.h"

#include "HealthComponent.h"
#include "PowerupComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Camera/CameraComponent.h"
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

	PowerupComponent = CreateDefaultSubobject<UPowerupComponent>(TEXT("PowerupComponent"));
	PowerupComponent->Owner = this;

	//Creates Audio Component for Engine or something...
	EngineAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioSource"));

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
	ShieldMesh->SetRelativeLocation({1600,0,-67});
	ShieldMesh->SetRelativeRotation({0,180,0});
	ShieldMesh->SetRelativeScale3D({5,5,5});
	ShieldMesh->SetVisibility(false);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetGenerateOverlapEvents(false);


}

void ABaseVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	InitAudio();
	InitVFX();
}

void ABaseVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateEngineSFX();
	UpdateGravelVFX();
	UpdateAirbornePhysics();
	
	//GEngine->AddOnScreenDebugMessage(-1, DeltaSeconds, FColor::Green, FString::Printf(TEXT("IS GROUNDED: %d"), IsGrounded()));
	//GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Green, FString::Printf(TEXT("BOOST AMOUNT: %f"), Booster.BoostAmount)); //DEBUG FÖR BOOST AMOUNT
}

void ABaseVehiclePawn::OnBoostPressed()
{
	if(Booster.BoostAmount <= 0) return;
	BoostVfxNiagaraComponent->Activate(true);
	Booster.SetEnabled(true);
	if(bUseCrazyCamera)
	{
		APlayerController* PController = Cast<APlayerController>(GetController());
		if(BoostCameraShake != nullptr) PController->PlayerCameraManager->StartCameraShake(BoostCameraShake, 1);
	}
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
	VehicleMovementComp->SetMaxEngineTorque(BoostMaxTorque);
	VehicleMovementComp->SetThrottleInput(1);
	SetBoostAmount(FMath::Clamp(Booster.BoostAmount-BoostCost, 0.f, Booster.MaxBoostAmount));
	GetWorld()->GetTimerManager().SetTimer(Booster.BoostTimer, this, &ABaseVehiclePawn::OnBoosting, BoostConsumptionRate*UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), true);
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
		break;
	case 2:
		PowerupComponent->BoostPowerup(); //Pickup.BoostPowerup(); //Infinite boost
		break;
	case 3:
		PowerupComponent->OverheatPowerup(); //Pickup.OverheatPowerup(); //No overheat
		break;
	case 4:
		PowerupComponent->ShieldPowerup(); //Pickup.ShieldPowerup(); //skapar shield
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

USceneComponent* ABaseVehiclePawn::GetHomingTargetPoint() const
{
	return HomingTargetPoint;
}

void ABaseVehiclePawn::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if( OtherActor == this || OverlappedComp != BumperCollision ) { return; }
	ABaseVehiclePawn* OtherVehicle = Cast<ABaseVehiclePawn>(OtherActor);
	if(OtherVehicle && !OtherVehicle->GetIsDead())
	{
		const auto Speed = GetVehicleMovement()->GetForwardSpeed();
		if( Speed < 100.f ) { return; }
		GEngine->AddOnScreenDebugMessage(0, 3, FColor::Cyan, FString::Printf(TEXT("%f"),
			UGameplayStatics::ApplyDamage(
				OtherActor,
				FMath::Clamp(FMath::Clamp(Speed, 1.f, Speed) / BumperDamageDividedBy, 0.f, MaxBumperDamage),
				GetController(),
				this,
				nullptr
			))
		);
	}
}
