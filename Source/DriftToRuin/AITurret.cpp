#include "AITurret.h"

AAITurret::AAITurret()
{
	
}

void AAITurret::BeginPlay()
{
	Super::BeginPlay();
}

void AAITurret::UpdateTurretRotation()
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

void AAITurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UpdateTurretRotation();
}



