// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseProjectile.h"
#include "MinigunProjectile.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AMinigunProjectile : public ABaseProjectile
{
	GENERATED_BODY()

public:
	AMinigunProjectile();

private:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpusle, const FHitResult& Hit) override;
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
