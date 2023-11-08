// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HomingMissileLauncher.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API AHomingMissileLauncher : public ABaseWeapon
{
	GENERATED_BODY()

public:
	AHomingMissileLauncher();

	virtual void PullTrigger() override;
	virtual void ReleaseTrigger() override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
};
