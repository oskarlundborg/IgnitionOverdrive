// Fill out your copyright notice in the Description page of Project Settings.
/**
* @author Mihajlo Radotic
*   Base class of all projectile types in the game.
*   Includes generic components and functionality of all projectile types. 
**/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class URadialForceComponent;
class UProjectileMovementComponent;

UCLASS()
class DRIFTTORUIN_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseProjectile();

	UProjectileMovementComponent* GetProjectileMovementComponent();

	/*Blueprint implementable event that runs in blueprints when OnOverlap callback function is executed. Used for setting up e.g VFX and SFX*/
	UFUNCTION(BlueprintImplementableEvent)
	void ProjectileImpactSweepResult(const FHitResult& SweepResult);

	/*Blueprint implementable event that runs in blueprints when OnHit callback function is executed. Used for setting up e.g VFX and SFX*/
	UFUNCTION(BlueprintImplementableEvent)
	void ProjectileImpactHitResult(const FHitResult& HitResult);

protected:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpusle, const FHitResult& Hit);
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(Category = "Sound", EditDefaultsOnly, BlueprintReadOnly)
	UAudioComponent* WhizzingAudioComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Radial Force")
	URadialForceComponent* RadialForceComponent;

	UPROPERTY(Category = "VFX", EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraComponent* ProjectileVfxNiagaraComponent;
	UPROPERTY(Category = "VFX", EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* ProjectileVfxNiagaraSystem;
  
	void InitializeAudio();

public:	
	virtual void Tick(float DeltaTime) override;

};
