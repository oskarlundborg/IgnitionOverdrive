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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
