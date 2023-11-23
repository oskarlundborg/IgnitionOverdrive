// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class DRIFTTORUIN_API ABaseProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseProjectile();

	UProjectileMovementComponent* GetProjectileMovementComponent();

	UFUNCTION(BlueprintImplementableEvent)
	void ProjectileImpactSweepResult(const FHitResult& SweepResult);

protected:
	/*Projectile callback function for collision*/
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpusle, const FHitResult& Hit) {}
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	float Damage = 0.f;
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ProjectileMesh;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraComponent* ProjectileVfxNiagaraComponent;
	UPROPERTY(Category=VFX, EditDefaultsOnly, BlueprintReadOnly)
	class UNiagaraSystem* ProjectileVfxNiagaraSystem;

public:	
	virtual void Tick(float DeltaTime) override;

};
