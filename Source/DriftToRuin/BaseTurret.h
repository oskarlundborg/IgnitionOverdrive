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
	// Sets default values for this actor's properties
	ABaseTurret();
	
	USkeletalMeshComponent* GetTurretMesh() const;
protected:
	// Called when the game starts or when spawned
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
