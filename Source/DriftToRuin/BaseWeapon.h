// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UCLASS()
class DRIFTTORUIN_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	/*Sets default values for this actor's properties*/
	ABaseWeapon();

	/*Fire functionality based on input button started or completed*/
	virtual void PullTrigger() {}
	virtual void ReleaseTrigger() {}

	USkeletalMeshComponent* GetWeaponMesh() const;
	USceneComponent* GetProjectileSpawnPoint() const;
protected:
	//Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/*Holds a blueprint projectile class set from derived weapon class blueprint*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class ABaseProjectile> ProjectileClass;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* WeaponRoot;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* ProjectileSpawnPoint;
public:	
	//Called every frame
	virtual void Tick(float DeltaTime) override;

};
