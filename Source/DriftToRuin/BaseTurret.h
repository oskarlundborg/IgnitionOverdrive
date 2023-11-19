// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTurret.generated.h"

UCLASS()
class DRIFTTORUIN_API ABaseTurret : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseTurret();
	
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetTurretMesh() const;
protected:
	
	virtual void BeginPlay() override;

	/*Updates the rotation of the turret based on the controllers Yaw rotation*/
	virtual void UpdateTurretRotation() {}
private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* TurretRoot;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* TurretMesh;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
