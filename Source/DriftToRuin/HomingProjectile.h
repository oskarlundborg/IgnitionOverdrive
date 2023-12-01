// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "HomingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AHomingProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AHomingProjectile();

private:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpusle, const FHitResult& Hit) override;
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	FTimerDelegate DestructionDelegate;
	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "VFX Destruction", meta = (AllowPrivateAccess = "true"))
	float DestructionTime;
	
	void CheckIfTargetDied();
	void OnDestroy();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
