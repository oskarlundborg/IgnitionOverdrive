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
	ABaseWeapon();

	/*Fire functionality based on input action started or completed*/
	virtual void PullTrigger() {}
	virtual void ReleaseTrigger() {}

	USkeletalMeshComponent* GetWeaponMesh() const;
	USceneComponent* GetProjectileSpawnPoint() const;
protected:
	virtual void BeginPlay() override;
	
	/*Holds a blueprint projectile class set from derived weapon class blueprint*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TSubclassOf<class ABaseProjectile> ProjectileClass;

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* WeaponRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* ProjectileSpawnPoint;

protected:
	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraComponent* MuzzleFlashNiagaraComponent;
	
	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* MuzzleFlashNiagaraSystem;
public:	
	virtual void Tick(float DeltaTime) override;

};
