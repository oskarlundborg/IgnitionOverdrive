//Daniel Olsson - Enbart en annan class för turret som används till AI, då AI implementering skiljer sig från spelare.
//Mihajlo Radotic - Står för implementering av Turret, då denna klass ärver från base turret, se Base Turret

#pragma once

#include "CoreMinimal.h"
#include "BaseTurret.h"
#include "AITurret.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AAITurret : public ABaseTurret
{
	GENERATED_BODY()

public:
	AAITurret();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	/*Updates the rotation of the turret based on the controllers Yaw rotation*/
	virtual void UpdateTurretRotation() override;
	
};
